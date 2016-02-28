#ifndef ATHVIEWS_ATHEVENTCONTEXT_H
#define ATHVIEWS_ATHEVENTCONTEXT_H

#include "GaudiKernel/EventContext.h"
#include "AthViews/View.h"

/** @class AthEventContext AthEventContext.h GaudiKernel/AthEventContext.h
 *
 * @author Ben Wynne
 * @date 2015
 **/


class AthEventContext : public EventContext
{
	public:

		AthEventContext() : EventContext(), m_eventView(0){};

		AthEventContext( ContextEvt_t const& ie, ContextID_t const& s = INVALID_CONTEXT_ID ) : EventContext( ie, s ), m_eventView(0){};

		AthEventContext( EventContext const* c )
		{
			this->setEvt( c->evt() );
			this->setSlot( c->slot() );
			this->setValid( c->valid() );
			this->setFail( c->evtFail() );
			m_eventView = 0;
		}

		virtual ~AthEventContext(){};

		virtual AthEventContext & operator = ( AthEventContext const& c )
		{
			this->setEvt( c.evt() );
			this->setSlot( c.slot() );
			this->setValid( c.valid() );
			this->setFail( c.evtFail() );
			m_eventView = c.m_eventView;
			return *this;
		}

		virtual AthEventContext & operator = ( EventContext const& c )
		{
			this->setEvt( c.evt() );
			this->setSlot( c.slot() );
			this->setValid( c.valid() );
			this->setFail( c.evtFail() );
			m_eventView = 0;
			return *this;
		}

		virtual void setView( SG::View * InputView ){ m_eventView = InputView; }
		virtual SG::View * getView() const { return m_eventView; } //TODO fix for const correctness

	private:
		SG::View * m_eventView;
};

inline std::ostream& operator<<( std::ostream& os, const AthEventContext& ctx ) {
	if (ctx.valid()) {
		return os << "s: " << ctx.slot() << "  e: " << ctx.evt() << " v: " << ctx.getView();
	} else {
		return os << "INVALID";
	}
}

inline std::ostream& operator<< (std::ostream& os, const AthEventContext* c) {
	if (c != 0) {
		return os << *c;
	} else {
		return os << "INVALID";
	}
}

#endif //ATHVIEWS_ATHEVENTCONTEXT_H
