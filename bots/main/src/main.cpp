// Entry point: draft instance picks from draft.cpp, champions run Bot. A failed turn is logged and still yields.
#include <cambc.h>
#include <cstdio>
#include "core/bot.h"
#include "draft/draft.h"

int main() {
    if (cambc::is_draft()) {
        while (true) {
            try {
                play_draft();
            } catch (const std::exception& e) {
                std::printf("draft error r%u: %s\n", cambc::round_number(), e.what());
            }
            cambc::end_turn();
        }
    }
    Bot bot;
    while (true) {
        try {
            bot.play_turn();
        } catch (const std::exception& e) {
            std::printf("error r%u act=%s: %s\n", cambc::round_number(), bot.current_act(), e.what());
        } catch (...) {
            std::printf("error r%u: unknown exception\n", cambc::round_number());
        }
        std::fflush(stdout);
        cambc::end_turn();
    }
}
