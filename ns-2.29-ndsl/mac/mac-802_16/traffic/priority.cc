#include "priority.h"
#include "agent.h"
#include "packet.h"

void priority::setpriority(Agent* agent, int priority){
packet_t pt;

//�]�w�Ȧstype
if(priority == 1){
pt = PT_UGS;
}
else if(priority == 2){
pt = PT_ertPS;
}
else if(priority == 3){
pt = PT_rtPS;
}
else if(priority == 4){
pt = PT_nrtPS;
}
else if(priority == 2){
pt = PT_BE;
}

//��ڳ]�wpacket type
agent->set_pkttype(pt);
}