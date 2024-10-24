# .gdbinit inside the project folder

# Enable auto-loading of Python scripts in the project directory
add-auto-load-safe-path .

# Add the current directory to the Python sys.path
python
import sys
sys.path.insert(0, ".")  # Add current directory to sys.path if not already
end


# Register type summaries for Tensor using the pretty_print function from tensor.py
python
import gdb
import re
from tools.pretty_print_lldb.parse_tensor import parse_string

import struct
import sys, os

render_outer_limit = os.environ["RL_TOOLS_LLDB_RENDER_OUTER_LIMIT"] if "RL_TOOLS_LLDB_RENDER_OUTER_LIMIT" in os.environ else 20
print(f"RLtools Tensor renderer outer limit: {render_outer_limit}")
render_inner_limit = os.environ["RL_TOOLS_LLDB_RENDER_INNER_LIMIT"] if "RL_TOOLS_LLDB_RENDER_INNER_LIMIT" in os.environ else 500
print(f"RLtools Tensor renderer inner limit: {render_inner_limit}")

def pad_number(number, length):
    return str(number).rjust(length)

def render(target, float_type, ptr, shape, stride, title="", use_title=False, outer_limit=render_outer_limit, inner_limit=render_inner_limit, base_offset=0):
    if len(shape) == 1:
        output = "["
        for element_i in range(shape[0]):
            pos = element_i * stride[0]
            offset = int(ptr) + base_offset + pos * float_type.size
            print(f"pointer: {int(ptr)}, base_offset: {base_offset}, pos: {pos} offset: {offset} type size: {float_type.sizeof}")

            memory = inferior.read_memory(offset, float_type.sizeof)

            dtype = 'f' if float_type.code == gdb.TYPE_CODE_FLT and float_type.name == "float" else 'd'
            val = struct.unpack(dtype, memory)[0]
            output += str(val) + ", "
        if shape[0] > 0:
            output = output[:-2]
        return output + "]"
    elif len(shape) == 2:
        output = "[ " + (("\\\\ Subtensor: " + title + f"({shape[0]}x{shape[1]})") if use_title and len(title) > 0 else "") + "\n"
        for row_i in range(shape[0]) if shape[0] < inner_limit else list(range(inner_limit // 2)) + ["..."] + list(range(shape[0] - inner_limit // 2, shape[0])):
            if row_i == "...":
                output += "...\n"
                continue
            output += "[" if shape[0] < inner_limit else f"{row_i}: ["
            for col_i in range(shape[1]) if shape[1] < inner_limit else list(range(inner_limit // 2)) + ["..."] + list(range(shape[1] - inner_limit // 2, shape[1])):
                if col_i == "...":
                    output += "..., "
                    continue
                pos = row_i * stride[0] + col_i * stride[1]
                offset = int(ptr) + base_offset + pos * float_type.sizeof
                print(f"pointer: {int(ptr)}, base_offset: {base_offset}, pos: {pos} offset: {offset} type size: {float_type.sizeof}")
                memory = target.read_memory(offset, float_type.sizeof)
                dtype = 'f' if float_type.code == gdb.TYPE_CODE_FLT and float_type.name == "float" else 'd'
                val = struct.unpack(dtype, memory)[0]
                output += str(val) + ", "
            if shape[1] > 0:
                output = output[:-2]
            output += "], \n"
        if shape[0] > 0:
            output = output[:-3]
            output += "\n"
        return output + "]\n"
    else:
        output = "[" + ("\n" if len(shape) == 3 else "")
        for i in range(shape[0]) if shape[0] < outer_limit else list(range(outer_limit // 2)) + ["..."] + list(range(shape[0] - outer_limit // 2, shape[0])):
            if i != "...":
                current_title = title + pad_number(i, 10) + " | "
                new_base_offset = base_offset + i * stride[0] * float_type.sizeof
                output += render(target, float_type, ptr, shape[1:], stride[1:], title=current_title, use_title=use_title, base_offset=new_base_offset) + ", \n"
            else:
                output += "...\n"
                output += "...\n"
                output += "...\n"
        if shape[0] > 0:
            output = output[:-3]
        return output + "]"


class TensorPrinter:
    """Pretty-printer for rl_tools::Tensor types."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        float_ptr = self.val["_data"]
        float_type = float_ptr.type.target()
        target = gdb.selected_inferior()

        typename = str(gdb.types.get_basic_type(self.val.type))
        print(f"float_type is: {float_type}")
        print(f"Typename is: {typename}")

        tensor = parse_string(typename)
        if tensor is None:
            print(f"Parse error on: {typename}")
            return typename
        else:
            return str(tensor) + "\n" + render(target, float_type, float_ptr, tensor.shape, tensor.stride)

# Register the Tensor pretty-printer for specific type patterns
def register_tensor_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("tensor_pretty_printer")
    pp.add_printer("Tensor", "^rl_tools::Tensor.*>$", TensorPrinter)
    pp.add_printer("Tensor", "^rl_tools::Tensor.*>\\s&\\s$", TensorPrinter)
    pp.add_printer("Tensor", "^const\\srl_tools::Tensor.*>$", TensorPrinter)
    pp.add_printer("Tensor", "^const\\srl_tools::Tensor.*>\\s&\\s$", TensorPrinter)
    gdb.printing.register_pretty_printer(gdb.current_objfile(), pp)

register_tensor_pretty_printer()
