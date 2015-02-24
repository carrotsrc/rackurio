/* Copyright 2015 Charlie Fyvie-Gauld
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "RackUnit.h"
using namespace RackoonIO;

RackUnit::RackUnit(string utype) {
	unitState = UNIT_OFF;
	rutype = utype;
}


void RackUnit::setName(string uname) {
	name = uname;
}

string RackUnit::getName() const {
	return name;
}

std::string RackUnit::getRuType() const {
	return rutype;
}
void RackUnit::setChain(RackChain *rchain) {
	chain = rchain;
}

RackChain *RackUnit::getChain() const {
	return chain;
}

void RackUnit::addJack(string jname, ConnectorType type) {
	Jack *jack;

	switch(type) {
	case JACK_AC:
	case JACK_SEQ:
		jack = (Jack*)new SeqJack(this);
		break;
	}

	jack->name = jname;
	jackArray.push_back(jack);
}

void RackUnit::addPlug(string pname) {
	Plug *plug = new Plug(this);
	plug->name = pname;
	plugArray.push_back(plug);
}


Jack *RackUnit::getJack(string name) const {
	int sz = jackArray.size();
	for(int i = 0; i < sz; i++)
		if(jackArray[i]->name == name)
			return (jackArray[i]);

	return NULL;
}

Plug *RackUnit::getPlug(string name) const {
	int sz = plugArray.size();
	for(int i = 0; i < sz; i++)
		if(plugArray[i]->name == name)
			return (plugArray[i]);

	return NULL;
}

void RackUnit::setRackQueue(RackQueue *queue) {
	rackQueue = queue;
}

void RackUnit::setMessageFactory(GenericEventMessageFactory *factory) {
	messageFactory = factory;
}

void RackUnit::setEventLoop(EventLoop *loop) {
	eventLoop = loop;
}

void RackUnit::setCacheHandler(CacheHandler *handler) {
	cacheHandler = handler;
}

void RackUnit::join() {

}

void RackUnit::unjoin() {

}

RackState RackUnit::rackFeed(RackState state) {
	switch(state) {
	case RACK_AC:
		if(unitState == UNIT_ACTIVE) {
			if(cycle() == RACK_UNIT_FAILURE)
				return RACK_UNIT_FAILURE;

			break;
		}
	case RACK_RESET:
		if(init() == RACK_UNIT_FAILURE)
			return RACK_UNIT_FAILURE;

		unitState = UNIT_ACTIVE;
		break;
	
	case RACK_OFF:
		unitState = UNIT_OFF;
		break;
	}

	int sz = plugArray.size();
	for(int i = 0; i < sz; i++) {
		if(plugArray[i]->connected &&
		plugArray[i]->jack->rackFeed(state) == RACK_UNIT_FAILURE)
			return RACK_UNIT_FAILURE;
	}

	return RACK_UNIT_OK;
}

void RackUnit::setConnection(string plug, string jack, RackUnit *unit) {
	Plug *p = this->getPlug(plug);
	Jack *j= unit->getJack(jack);
	p->jack = j;
	p->connected = true;
	j->connected = true;
	cout << getName() << ":" << p->name << " -> "
	<< unit->getName() << ":" << j->name << endl;
}

void RackUnit::outsource(std::function<void()> run) {
	rackQueue->addPackage(run);
}


void RackUnit::midiExportMethod(string action, std::function<void(int)> method) {
	midiExport.insert( std::pair<string, std::function<void(int)> >(action, method) );
}

bool RackUnit::midiControllable()
{
	if(midiExport.size() == 0)
		return false;

	return true;
}

std::map<string, std::function<void(int)> > RackUnit::midiExportedMethods(){ 
	return midiExport; 
}

void RackUnit::addEvent(std::unique_ptr<EventMessage> msg) {
	eventLoop->addEvent(std::move(msg));
}

std::unique_ptr<EventMessage> RackUnit::createMessage(EventType type) {
	return std::move(messageFactory->createMessage(type));
}

void RackUnit::addEventListener(EventType type, std::function<void(shared_ptr<EventMessage>)> callback) {
	eventLoop->addEventListener(type, callback);
}


short *RackUnit::cacheAlloc(int num) {
	return cacheHandler->alloc(num);
}

void RackUnit::cacheFree(short *mem) {
	return cacheHandler->free(mem);
}
