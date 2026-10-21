#include <cambc.h>
#include <cstdio>
#include "bot.h"
#include "draft.h"
int main(){if(cambc::is_draft()){while(true){play_draft();cambc::end_turn();}}Bot b;b.setup();auto& me=cambc::self();std::printf("DRAFT_PROBE kind=%d slot=%d team_size=%d picks=%zu handle=%u\n",static_cast<int>(me.champion_name()),b.mem_slot,b.team_size,cambc::draft::current_draft(cambc::team::ally).size(),static_cast<unsigned>(static_cast<cambc::handle_champion_t>(me)));while(true)cambc::end_turn();}
