/**************************1*************************************************
                          DocClipRef.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Jason Wood
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

#include <kdebug.h>

#include "clipmanager.h"
#include "docclipbase.h"
#include "docclipavfile.h"
#include "docclipproject.h"
#include "doctrackbase.h"
#include "docclipbase.h"
#include "effectdesc.h"
#include "kdenlivedoc.h"

DocClipRef::DocClipRef(DocClipBase *clip) :
	m_trackStart(0.0),
	m_cropStart(0.0),
	m_trackEnd(0.0),
	m_parentTrack(0),
	m_trackNum(-1),
	m_clip(clip)
{
	if(!clip) {
		kdError() << "Creating a DocClipRef with no clip - not a very clever thing to do!!!" << endl;
	}
}

DocClipRef::~DocClipRef()
{
}

const GenTime &DocClipRef::trackStart() const {
  return m_trackStart;
}

void DocClipRef::setTrackStart(const GenTime time)
{
	m_trackStart = time;

	if(m_parentTrack) {
		m_parentTrack->clipMoved(this);
	}
}

const QString &DocClipRef::name() const
{
	return m_clip->name();
}

const QString &DocClipRef::description() const
{
	return m_clip->description();
}

void DocClipRef::setDescription(const QString &description)
{
	m_clip->setDescription(description);
}

void DocClipRef::setCropStartTime(const GenTime &time)
{
	m_cropStart = time;
	if(m_parentTrack) {
		m_parentTrack->clipMoved(this);
	}
}

const GenTime &DocClipRef::cropStartTime() const
{
	return m_cropStart;
}

void DocClipRef::setTrackEnd(const GenTime &time)
{
	m_trackEnd = time;

	if(m_parentTrack) {
		m_parentTrack->clipMoved(this);
	}
}

GenTime DocClipRef::cropDuration() const
{
	return m_trackEnd - m_trackStart;
}

DocClipRef *DocClipRef::createClip(const EffectDescriptionList &effectList, ClipManager &clipManager, const QDomElement &element)
{
	DocClipRef *clip = 0;
	GenTime trackStart;
	GenTime cropStart;
	GenTime cropDuration;
	GenTime trackEnd;
	QString description;
	QValueVector<GenTime> markers;
	EffectStack effectStack;

	int trackNum = 0;

	QDomNode node = element;
	node.normalize();

	if(element.tagName() != "clip") {
		kdWarning()	<< "DocClipRef::createClip() element has unknown tagName : " << element.tagName() << endl;
		return 0;
	}

	DocClipBase *baseClip = clipManager.insertClip(element);

	if(baseClip) {
		clip = new DocClipRef(baseClip);
	}

	QDomNode n = element.firstChild();

	while(!n.isNull()) {
		QDomElement e = n.toElement();
		if(!e.isNull()) {
			/**if(e.tagName() == "avfile") {
				clip = DocClipAVFile::createClip(e);
			} else if(e.tagName() == "project") {
				clip = DocClipProject::createClip(e);
			} else */
			if(e.tagName() == "position") {
				trackNum = e.attribute("track", "-1").toInt();
				trackStart = GenTime(e.attribute("trackstart", "0").toDouble());
				cropStart = GenTime(e.attribute("cropstart", "0").toDouble());
				cropDuration = GenTime(e.attribute("cropduration", "0").toDouble());
				trackEnd = GenTime(e.attribute("trackend", "-1").toDouble());
			}

			if(e.tagName() == "markers") {
				QDomNode markerNode = e.firstChild();
				while(!markerNode.isNull()) {
					QDomElement markerElement = markerNode.toElement();
					if(!markerElement.isNull()) {
						if(markerElement.tagName() == "marker") {
							markers.append(GenTime(markerElement.attribute("time", "0").toDouble()));
						} else {
							kdWarning() << "Unknown tag " << markerElement.tagName() << endl;
						}
					}
					markerNode = markerNode.nextSibling();
				}
			}

			if(e.tagName() == "effects") {
				QDomNode effectNode = e.firstChild();
				while(!effectNode.isNull()) {
					QDomElement effectElement = effectNode.toElement();
					if(!effectElement.isNull()) {
						if(effectElement.tagName() == "effect") {
							EffectDesc *desc = effectList.effectDescription(effectElement.attribute("type", QString::null));
							if(desc) {
								kdWarning() << "Appending effect " << desc->name() << endl;
								effectStack.append(Effect::createEffect(*desc, effectElement));
							} else {
								kdWarning() << "Unknown effect " << effectElement.attribute("type", QString::null) << endl;
							}
						} else {
							kdWarning() << "Unknown tag " << effectElement.tagName() << endl;
						}
					}
					effectNode = effectNode.nextSibling();
				}
			}
		}

		n = n.nextSibling();
	}

	if(clip==0) {
	  kdWarning()	<< "DocClipRef::createClip() unable to create clip" << endl;
	} else {
		// setup DocClipRef specifics of the clip.
		clip->setTrackStart(trackStart);
		clip->setCropStartTime(cropStart);
		if(trackEnd.seconds() != -1) {
			clip->setTrackEnd(trackEnd);
		} else {
			clip->setTrackEnd(trackStart + cropDuration);
		}
		clip->setParentTrack(0, trackNum);
		clip->setSnapMarkers(markers);
		//clip->setDescription(description);
		clip->setEffectStack(effectStack);
	}

	return clip;
}

/** Sets the parent track for this clip. */
void DocClipRef::setParentTrack(DocTrackBase *track, const int trackNum)
{
	m_parentTrack = track;
	m_trackNum = trackNum;
}

/** Returns the track number. This is a hint as to which track the clip is on, or should be placed on. */
int DocClipRef::trackNum() const
{
	return m_trackNum;
}

/** Returns the end of the clip on the track. A convenience function, equivalent
to trackStart() + cropDuration() */
GenTime DocClipRef::trackEnd() const
{
	return m_trackEnd;
}

/** Returns the parentTrack of this clip. */
DocTrackBase * DocClipRef::parentTrack()
{
	return m_parentTrack;
}

/** Move the clips so that it's trackStart coincides with the time specified. */
void DocClipRef::moveTrackStart(const GenTime &time)
{
	m_trackEnd = m_trackEnd + time - m_trackStart;
	m_trackStart = time;
}

DocClipRef *DocClipRef::clone(const EffectDescriptionList &effectList, ClipManager &clipManager)
{
	return createClip(effectList, clipManager, toXML().documentElement());
}

bool DocClipRef::referencesClip(DocClipBase *clip) const
{
	return m_clip->referencesClip(clip);
}

uint DocClipRef::numReferences() const
{
	return m_clip->numReferences();
}

double DocClipRef::framesPerSecond() const
{
	if(m_parentTrack) {
		return m_parentTrack->framesPerSecond();
	} else {
		return m_clip->framesPerSecond();
	}
}

QDomDocument DocClipRef::toXML() const
{
	QDomDocument doc = m_clip->toXML();

	QDomElement clip = doc.documentElement();
	if(clip.tagName() != "clip") {
		kdError() << "Expected tagname of 'clip' in DocClipRef::toXML(), expect things to go wrong!" << endl;
	}

	QDomElement position = doc.createElement("position");
	position.setAttribute("track", QString::number(trackNum()));
	position.setAttribute("trackstart", QString::number(trackStart().seconds(), 'f', 10));
	position.setAttribute("cropstart", QString::number(cropStartTime().seconds(), 'f', 10));
	position.setAttribute("cropduration", QString::number(cropDuration().seconds(), 'f', 10));
	position.setAttribute("trackend", QString::number(trackEnd().seconds(), 'f', 10));

	clip.appendChild(position);

	if(!m_effectStack.isEmpty()) {
		QDomElement effects = doc.createElement("effects");

		EffectStack::iterator itt = m_effectStack.begin();
		while(itt != m_effectStack.end()) {
			effects.appendChild( (*itt)->toXML());
			++itt;
		}

		clip.appendChild(effects);
	}

	QDomElement markers = doc.createElement("markers");
	for(uint count=0; count<m_snapMarkers.count(); ++count) {
		QDomElement marker = doc.createElement("marker");
		marker.setAttribute("time", QString::number(m_snapMarkers[count].seconds(), 'f', 10));
		markers.appendChild(marker);
	}
	clip.appendChild(markers);

	doc.appendChild(clip);

	return doc;
}

bool DocClipRef::matchesXML(const QDomElement &element) const
{
	bool result = false;
	if(element.tagName() == "clip")
	{
		QDomNodeList nodeList = element.elementsByTagName("position");
		if(nodeList.length() > 0) {
			if(nodeList.length() > 1) {
				kdWarning() << "More than one position in XML for a docClip? Only matching first one" << endl;
			}
			QDomElement positionElement = nodeList.item(0).toElement();

			if(!positionElement.isNull()) {
				result = true;
				if(positionElement.attribute("track").toInt() != trackNum()) result = false;
				if(positionElement.attribute("trackstart").toInt() != trackStart().seconds()) result = false;
				if(positionElement.attribute("cropstart").toInt() != cropStartTime().seconds()) result = false;
				if(positionElement.attribute("cropduration").toInt() != cropDuration().seconds()) result = false;
				if(positionElement.attribute("trackend").toInt() != trackEnd().seconds()) result = false;
			}
		}
	}

	return result;
}

const GenTime &DocClipRef::duration() const
{
	return m_clip->duration();
}

bool DocClipRef::durationKnown() const
{
	return m_clip->durationKnown();
}

QDomDocument DocClipRef::generateSceneList()
{
#warning - this may not be correct.
	return m_clip->generateSceneList();
}

const KURL &DocClipRef::fileURL() const
{
	return m_clip->fileURL();
}

uint DocClipRef::fileSize() const
{
	return m_clip->fileSize();
}

void DocClipRef::populateSceneTimes(QValueVector<GenTime> &toPopulate)
{
	QValueVector<GenTime> sceneTimes;

	m_clip->populateSceneTimes(sceneTimes);

	QValueVector<GenTime>::iterator itt = sceneTimes.begin();
	while(itt != sceneTimes.end())
	{
		GenTime convertedTime = (*itt) - cropStartTime();
		if((convertedTime >= GenTime(0)) && (convertedTime <= cropDuration())) {
			convertedTime += trackStart();
			toPopulate.append(convertedTime);
		}

		++itt;
	}

	toPopulate.append(trackStart());
	toPopulate.append(trackEnd());
}

QDomDocument DocClipRef::sceneToXML(const GenTime &startTime, const GenTime &endTime)
{
	QDomDocument sceneDoc;

	if(m_effectStack.isEmpty()) {
		sceneDoc = m_clip->sceneToXML(startTime - trackStart() + cropStartTime(), endTime - trackStart() + cropStartTime());
	} else {
		// We traverse the effect stack backwards; this let's us add the video clip at the top of the effect stack.
		QDomNode constructedNode = sceneDoc.importNode(m_clip->sceneToXML(startTime - trackStart() + cropStartTime(), endTime - trackStart() + cropStartTime()).documentElement(), true);
		EffectStackIterator itt(m_effectStack);
		itt.toLast();

		while(itt.current()) {
			QDomElement parElement = sceneDoc.createElement("par");
			parElement.appendChild(constructedNode);

			QDomElement effectElement = sceneDoc.createElement("videoeffect");
			effectElement.setAttribute("name", (*itt)->effectDescription().name());
			effectElement.setAttribute("begin", QString::number(startTime.seconds(), 'f', 10));
			effectElement.setAttribute("dur", QString::number((endTime - startTime).seconds(), 'f', 10));
			parElement.appendChild(effectElement);

			constructedNode = parElement;
			--itt;
		}
		sceneDoc.appendChild(constructedNode);
	}

	return sceneDoc;
}

void DocClipRef::setSnapMarkers(QValueVector<GenTime> markers)
{
	m_snapMarkers = markers;
	qHeapSort(m_snapMarkers);

	/*QValueVectorIterator<GenTime> itt = markers.begin();

	while(itt != markers.end()) {
		addSnapMarker(*itt);
		++itt;
	}*/
}

QValueVector<GenTime> DocClipRef::snapMarkersOnTrack() const
{
	QValueVector<GenTime> markers;
	markers.reserve(m_snapMarkers.count());

	for(uint count=0; count<m_snapMarkers.count(); ++count) {
		markers.append( m_snapMarkers[count] + trackStart() - cropStartTime() );
	}

	return markers;
}

void DocClipRef::addSnapMarker(const GenTime &time)
{
	QValueVector<GenTime>::Iterator itt = m_snapMarkers.begin();

	while(itt != m_snapMarkers.end()) {
		if(*itt >= time) break;
		++itt;
	}

	if((itt != m_snapMarkers.end()) && (*itt == time)) {
		kdError() << "trying to add Snap Marker that already exists, this will cause inconsistancies with undo/redo" << endl;
	} else {
		m_snapMarkers.insert(itt, time);

		m_parentTrack->notifyClipChanged(this);
	}
}

void DocClipRef::deleteSnapMarker(const GenTime &time)
{
	QValueVector<GenTime>::Iterator itt = m_snapMarkers.begin();

	while(itt != m_snapMarkers.end()) {
		if(*itt == time) break;
		++itt;
	}

	if((itt != m_snapMarkers.end()) && (*itt == time)) {
		m_snapMarkers.erase(itt);
		m_parentTrack->notifyClipChanged(this);
	} else {
		kdError() << "Could not delete marker at time " << time.seconds() << " - it doesn't exist!" << endl;
	}
}

bool DocClipRef::hasSnapMarker(const GenTime &time)
{
	QValueVector<GenTime>::Iterator itt = m_snapMarkers.begin();

	while(itt != m_snapMarkers.end()) {
		if(*itt == time) return true;
		++itt;
	}

	return false;
}

void DocClipRef::setCropDuration(const GenTime &time)
{
	setTrackEnd( trackStart() + time );
}

GenTime DocClipRef::findPreviousSnapMarker(const GenTime &time)
{
	QValueVector<GenTime>::Iterator itt = m_snapMarkers.begin();

	while(itt != m_snapMarkers.end()) {
		if(*itt >= time) break;
		++itt;
	}

	if(itt != m_snapMarkers.begin()) {
		--itt;
		return *itt;
	} else {
		return GenTime(0);
	}
}

GenTime DocClipRef::findNextSnapMarker(const GenTime &time)
{
	QValueVector<GenTime>::Iterator itt = m_snapMarkers.begin();

	while(itt != m_snapMarkers.end()) {
		if(time <= *itt) break;
		++itt;
	}

	if( itt != m_snapMarkers.end())
	{
		if(*itt == time) {
			++itt;
		}
		if( itt != m_snapMarkers.end()) {
			return *itt;
		} else {
			return GenTime(duration());
		}
	} else {
		return GenTime(duration());
	}
}

GenTime DocClipRef::trackMiddleTime() const
{
	return (m_trackStart + m_trackEnd)/2;
}

QValueVector<GenTime> DocClipRef::snapMarkers() const
{
	return m_snapMarkers;
}

void DocClipRef::addEffect(uint index, Effect *effect)
{
	m_effectStack.insert(index, effect);
}

void DocClipRef::deleteEffect(uint index)
{
	m_effectStack.remove(index);
}

Effect *DocClipRef::effectAt(uint index) const
{
	EffectStack::iterator itt = m_effectStack.begin();

	if(index < m_effectStack.count()) {
		for(uint count=0; count<index; ++count) ++itt;
		return *itt;
	}

	kdError() << "DocClipRef::effectAt() : index out of range" << endl;
	return 0;

}

void DocClipRef::setEffectStack(const EffectStack &effectStack)
{
	m_effectStack = effectStack;
}
