/***************************************************************************
                          trackpanelfunction.h  -  description
                             -------------------
    begin                : Sun May 18 2003
    copyright            : (C) 2003 by Jason Wood
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
#ifndef TRACKPANELFUNCTION_H
#define TRACKPANELFUNCTION_H

#include "qcursor.h"
#include "qobject.h"

#include "kdenlive.h"

class QMouseEvent;

class KTrackPanel;

/**
Abstract Base Class for track panel functionality decorators. This and it's
derived classes allow different behaviours to be added to panels as required.

@author Jason Wood
*/
class TrackPanelFunction : public QObject
{
	Q_OBJECT
public:
	TrackPanelFunction();

    virtual ~TrackPanelFunction();

	/**
	Returns true if the specified position should cause this function to activate,
	otherwise returns false.
	*/
	virtual bool mouseApplies(KTrackPanel *panel, QMouseEvent *event) const = 0;

	/**
	Returns a relevant mouse cursor for the given mouse position
	*/
	virtual QCursor getMouseCursor(KTrackPanel *panel, QMouseEvent *event) = 0;

	/**
	A mouse button has been pressed. Returns true if we want to handle this event
	*/
	virtual bool mousePressed(KTrackPanel *panel, QMouseEvent *event) = 0;

	/**
	Mouse Release Events in the track view area. Returns true if we have finished
	an operation now.
	*/
	virtual bool mouseReleased(KTrackPanel *panel, QMouseEvent *event) = 0;

	/**
	Processes Mouse Move events in the track view area. Returns true if we are
	continuing with the drag.*/
	virtual bool mouseMoved(KTrackPanel *panel, QMouseEvent *event) = 0;

	/**
	Process Drag events*/
    	virtual bool dragEntered ( KTrackPanel *panel, QDragEnterEvent * ) { return false; };
    	virtual bool dragMoved ( KTrackPanel *panel, QDragMoveEvent * ) { return false; };
    	virtual bool dragLeft ( KTrackPanel *panel, QDragLeaveEvent * ) { return false; };
    	virtual bool dragDropped ( KTrackPanel *panel, QDropEvent * ) { return false; };

};

#endif
