# Compiling Python source costs ~44k fuel per line in this runtime, so gameplay code is
# loaded from precompiled bytecode (src/bot.bin, made by build_bytecode.py with CPython 3.13),
# falling back to bot.py if that fails. The draft module is tiny and compiles from source.
import cambc, sys


def load_bot():
    try:
        import marshal, types
        if sys.version_info[:2] == (3, 13):
            with open('/guest/bot.bin', 'rb') as f:
                code = marshal.loads(f.read())
            mod = types.ModuleType('bot')
            exec(code, mod.__dict__)
            return mod
    except Exception as e:
        print('bytecode fallback:', repr(e), flush=True)
    import bot
    return bot


def main():
    if cambc.is_draft():
        import draft
        play = draft.play
    else:
        play = load_bot().Bot().play_turn
    while True:
        try:
            play()
        except Exception as e:
            import traceback
            print('error r%d: %r %s' % (cambc.round_number(), e, traceback.format_exc().splitlines()[-4:-1]), flush=True)
        cambc.end_turn()


main()
