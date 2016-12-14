/* Copyright (c) 2013-2016 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "timing.h"

void mTimingInit(struct mTiming* timing, int32_t* relativeCycles, int32_t* nextEvent) {
	timing->root = NULL;
	timing->masterCycles = 0;
	timing->relativeCycles = relativeCycles;
	timing->nextEvent = nextEvent;
}

void mTimingDeinit(struct mTiming* timing) {
}

void mTimingClear(struct mTiming* timing) {
	timing->root = NULL;
	timing->masterCycles = 0;
}

void mTimingSchedule(struct mTiming* timing, struct mTimingEvent* event, int32_t when) {
	int32_t nextEvent = when + *timing->relativeCycles;
	event->when = nextEvent + timing->masterCycles;
	if (nextEvent < *timing->nextEvent) {
		*timing->nextEvent = nextEvent;
	}
	struct mTimingEvent** previous = &timing->root;
	struct mTimingEvent* next = timing->root;
	while (next) {
		int32_t nextWhen = next->when - timing->masterCycles;
		if (nextWhen > when) {
			break;
		}
		previous = &next->next;
		next = next->next;
	}
	event->next = next;
	*previous = event;
}

void mTimingDeschedule(struct mTiming* timing, struct mTimingEvent* event) {
	struct mTimingEvent** previous = &timing->root;
	struct mTimingEvent* next = timing->root;
	while (next) {
		if (next == event) {
			*previous = next->next;
			return;
		}
		previous = &next->next;
		next = next->next;
	}
}

int32_t mTimingTick(struct mTiming* timing, int32_t cycles) {
	timing->masterCycles += cycles;
	uint32_t masterCycles = timing->masterCycles;
	while (timing->root) {
		struct mTimingEvent* next = timing->root;
		int32_t nextWhen = next->when - masterCycles;
		if (nextWhen > 0) {
			return nextWhen;
		}
		timing->root = next->next;
		next->callback(timing, next->context, -nextWhen);
	}
	return *timing->nextEvent;
}

int32_t mTimingCurrentTime(struct mTiming* timing) {
	return timing->masterCycles + *timing->relativeCycles;
}

int32_t mTimingNextEvent(struct mTiming* timing) {
	struct mTimingEvent* next = timing->root;
	if (!next) {
		return INT_MAX;
	}
	return next->when - timing->masterCycles;
}