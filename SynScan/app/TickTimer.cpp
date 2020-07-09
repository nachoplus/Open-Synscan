/*
 * This file is part of the Open Synscan Project distribution (https://github.com/vsirvent/Synscan).
 * Copyright (c) 2019 Vicente Sirvent Orts.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TickTimer.h"


TickTimer AppTimer;

static void IRAM_ATTR tick_timer_isr_cb(void *arg)
{
	if (arg == null) {
		return;
	}
	((TickTimer*)arg)->onTick();
	return;
}

TickTimer::TickTimer() {
	//ETS_FRC_TIMER1_INTR_ATTACH(tick_timer_isr_cb,  this);
	hw_timer1_attach_interrupt(TIMER_FRC1_SOURCE, tick_timer_isr_cb,this );
}

TickTimer::~TickTimer() {
	//ETS_FRC_TIMER1_INTR_ATTACH(tick_timer_isr_cb, null);
	hw_timer1_detach_interrupt();
	//hw_timer1_attach_interrupt(TIMER_FRC1_SOURCE, tick_timer_isr_cb,null );
	stop();
}

void IRAM_ATTR
TickTimer::onTick() {
	int size = mListener.size();
	for (int i = 0; i < size; ++i) {
		mListener[i]->onTick();
	}
}

bool TickTimer::start(bool repeating) {
	this->mRepeating = repeating;
	stop();
	if (mInternal == 0) {
		return mStarted;
	}

	if (this->mRepeating == 1) {
		hw_timer1_enable(TIMER_CLKDIV_16,TIMER_EDGE_INT,true);
	} else {
		hw_timer1_enable(TIMER_CLKDIV_16,TIMER_EDGE_INT,false);
	}

	//TM1_EDGE_INT_ENABLE();
	//ETS_FRC1_INTR_ENABLE();
	mStarted = true;

	hw_timer1_write(mInternal);
	return mStarted;
}

bool TickTimer::stop() {
	if (!mStarted) {
		return mStarted;
	}
	//TM1_EDGE_INT_DISABLE();
	//ETS_FRC1_INTR_DISABLE();
	mStarted = false;
	return mStarted;
}

bool TickTimer::restart() {
	stop();
	start(mRepeating);
	return mStarted;
}

bool TickTimer::isStarted() {
	return mStarted;
}

uint32_t TickTimer::getIntervalTicks() const {
	return mInternal;
}

bool TickTimer::setIntervalTicks(uint32_t ticks) {
	if (ticks > 0) {
		mInternal = ticks;
		if (mStarted) {
			restart();
		}
	} else {
		stop();
	}
	return mStarted;
}

void TickTimer::addListener(ITickListener* listener) {
	ETS_INTR_LOCK();
	if (!mListener.contains(listener)) {
		mListener.add(listener);
	}
	ETS_INTR_UNLOCK();
}

void TickTimer::removeListener(ITickListener* listener) {
	ETS_INTR_LOCK();
	mListener.removeElement(listener);
	ETS_INTR_UNLOCK();
}
