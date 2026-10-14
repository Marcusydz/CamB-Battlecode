"""Precompile src/bot.py to src/bot.bin. Run with CPython 3.13 after every edit to bot.py."""
import marshal, pathlib, sys
assert sys.version_info[:2] == (3, 13), "needs CPython 3.13 to match the game runtime"
src = pathlib.Path(__file__).parent / "src"
code = compile((src / "bot.py").read_text(), "bot.py", "exec", optimize=2)
(src / "bot.bin").write_bytes(marshal.dumps(code))
print("wrote", src / "bot.bin", (src / "bot.bin").stat().st_size, "bytes")
