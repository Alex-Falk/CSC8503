#include "Pursue.h"
#include "Find.h"
#include "Chase.h"

void Pursue::On_Initialize() {
	_isActive = true;
	h->SwitchState(PURSUE_CHASE);
}
