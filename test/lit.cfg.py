import os

import lit.formats
from lit.llvm import llvm_config

config.name = "CLANGIR_TAFFO"
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
config.suffixes = [".c", ".mlir"]
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.clangir_taffo_obj_root, "test")

llvm_config.use_default_substitutions()

config.excludes = [
    "CMakeLists.txt",
    "lit.cfg.py",
    "lit.site.cfg.py",
]

tool_dirs = [config.clangir_taffo_tools_dir, config.llvm_tools_dir]
llvm_config.add_tool_substitutions(
    ["cir-opt", "clang", "clangir-taffo-opt", "mlir-opt", "not"], tool_dirs
)
