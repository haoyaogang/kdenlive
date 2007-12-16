/***************************************************************************
                          KAddTransitionCommand  -  description
                             -------------------
    begin                : Thu Jan 22 2004
    copyright            : (C) 2004 by Jason Wood
    email                : jasonwood@blueyonder.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "kaddtransitioncommand.h"

#include <qdom.h>

#include <kdebug.h>
#include <klocale.h>

#include "docclipproject.h"
#include "docclipref.h"
#include "doctrackbase.h"
#include "transition.h"
#include "kdenlivedoc.h"

namespace Command {

// static
    KAddTransitionCommand *KAddTransitionCommand::appendTransition(KdenliveDoc * document, DocClipRef * clip, GenTime time, const QString & type) {
        Transition *transit = new Transition(clip, time, type);
	if (transit) return new KAddTransitionCommand(document, clip, transit, true);
    }

// static
    KAddTransitionCommand *KAddTransitionCommand::appendTransition(KdenliveDoc * document, DocClipRef * a_clip, DocClipRef * b_clip, const QString & type) {
        Transition *transit = new Transition(a_clip, b_clip, type);
	if (transit) return new KAddTransitionCommand(document, a_clip, transit, true);
    }

// static
    KAddTransitionCommand *KAddTransitionCommand::appendTransition(KdenliveDoc * document, DocClipRef * clip, Transition *transit) {
	return new KAddTransitionCommand(document, clip, transit, true);
    }

// static
    KAddTransitionCommand *KAddTransitionCommand::removeTransition(KdenliveDoc * document, DocClipRef * clip, Transition *transit) {
	return new KAddTransitionCommand(document, clip, transit, false);
    }

// static
/*
    KCommand *KAddTransitionCommand::moveTransition(KdenliveDoc * document,
	DocClipRef * clip, int effectIndex, int newEffectIndex) {
	KMacroCommand *command = new KMacroCommand(i18n("Move Effect"));

	command->addCommand(removeEffect(document, clip, effectIndex));
	command->addCommand(insertEffect(document, clip, newEffectIndex,
		clip->effectStack()[effectIndex]));
	return command;
    }*/

  KAddTransitionCommand::KAddTransitionCommand(KdenliveDoc * document, DocClipRef * clip, Transition *transit, bool add):
    KCommand(),
	m_addTransition(add), m_trackIndex(clip->trackNum()),
	m_position(clip->trackStart() + clip->cropDuration() / 2), 
	m_transition(transit->toXML()), m_document(document), m_transitionIndex(-1) {
    }


    KAddTransitionCommand::~KAddTransitionCommand() {
    }

// virtual
    QString KAddTransitionCommand::name() const {
	return m_addTransition ? i18n("Add transition") : i18n("Delete transition");
    } 

    void KAddTransitionCommand::execute() {
	if (m_addTransition) {
	    addTransition();
	} else {
	    deleteTransition();
	}
    }

    void KAddTransitionCommand::unexecute() {
	if (m_addTransition) {
	    deleteTransition();
	} else {
	    addTransition();
	}
    }

    void KAddTransitionCommand::addTransition() {
	DocTrackBase *track = m_document->projectClip().track(m_trackIndex);
	if (track) {
		DocClipRef *clip = track->getClipAt(m_position);
		if (!clip) return;
		Transition *tr = new Transition(clip, m_transition);
		m_transitionIndex = clip->addTransition(tr);

		m_document->renderer()->mltAddTransition(tr->transitionTag(), tr->transitionEndTrack(), tr->transitionStartTrack(), tr->transitionStartTime(), tr->transitionEndTime(), tr->transitionParameters());
	}
    }

    void KAddTransitionCommand::deleteTransition() {
	kdDebug()<<"//////// delete TRANSITION AT:"<<m_transitionIndex<<endl;
	DocTrackBase *track = m_document->projectClip().track(m_trackIndex);
	if (track) {
		DocClipRef *clip = track->getClipAt(m_position);
		if (!clip) return;
		if (m_transitionIndex != -1) {
		    m_transition = clip->transitionAtIndex(m_transitionIndex);
		    clip->deleteTransition(m_transitionIndex);
		}
		else clip->deleteTransition(m_transition);
		m_document->activateSceneListGeneration(true);
	}
    }

}
