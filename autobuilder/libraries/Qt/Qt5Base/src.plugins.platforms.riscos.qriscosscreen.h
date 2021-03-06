/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the RISC OS platform plugin of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/****************************************************************************
**
** Written by Lee Noar
**
****************************************************************************/

#ifndef QRISCOSSCREEN_H
#define QRISCOSSCREEN_H

#include <QWindow>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformnativeinterface.h>
#include "oslib/wimp.h"
#include "oslib/colourtrans.h"

QT_BEGIN_NAMESPACE

class QRiscosBackingStore;
class QRiscosWindow;
class QRiscosCursor;
class QDynamicArea;

class QRiscosScreen : public QPlatformScreen
{
public:
    QRiscosScreen();
    ~QRiscosScreen();

    // Called on a mode change to cache its values.
    void update();

    void addSurface(QRiscosBackingStore *);
    void removeSurface(QRiscosBackingStore *);

    QRiscosWindow *windowFromId(WId handle) const;

    QRiscosWindow *windowFromId(wimp_w handle) const {
	return windowFromId((WId)handle);
    }

    QRiscosBackingStore *surfaceFromId(WId handle) const;

    QRiscosBackingStore *surfaceFromId(wimp_w handle) const {
	return surfaceFromId((WId)handle);
    }

    // Overridden from base class
    QRect geometry() const Q_DECL_OVERRIDE { return mGeometry; }
    QSize size() const { return QSize(mGeometry.width(), mGeometry.height()); }
    int depth() const Q_DECL_OVERRIDE { return mDepth; }
    QImage::Format format() const Q_DECL_OVERRIDE { return mFormat; }

    QPlatformCursor *cursor() const Q_DECL_OVERRIDE;

    int xPixelToOS(int x) const {
	return x << mXEigenFactor;
    }
    int yPixelToOS(int y) const {
	return y << mYEigenFactor;
    }
    int xOSToPixel(int x) const {
	return x >> mXEigenFactor;
    }
    int yOSToPixel(int y) const {
	return y >> mYEigenFactor;
    }

    QSize sizeOS() const {
	return QSize(xPixelToOS(mGeometry.width()),
		     yPixelToOS(mGeometry.height()));
    }

    int heightOS() const {
	return yPixelToOS(mGeometry.height());
    }

    int widthOS() const {
	return xPixelToOS(mGeometry.width());
    }

    // Pixels in
    // Pixels out
    QPoint globalPoint(const QPoint &point) {
	return QPoint(point.x(),
		      mGeometry.height() - point.y());
    }

/*    QPoint osToPixel(int x, int y) {
	return QPoint(xOSToPixel(x), yOSToPixel(y));
    }
    QPoint pixelToOS(int x, int y) {
	return QPoint(xPixelToOS(x), yPixelToOS(y));
    }*/

    QSize osSizeToPixel(int x, int y) {
	return QSize(xOSToPixel(x), yOSToPixel(y));
    }
    QPoint osPointToPixel(const QPoint &point) {
	return QPoint(xOSToPixel(point.x()), yOSToPixel(point.y()));
    }
    QPoint osPointToPixel(int x, int y) {
	return QPoint(xOSToPixel(x), yOSToPixel(y));
    }
    QPoint pixelPointToOS(const QPoint &point) {
	return QPoint(xPixelToOS(point.x()), yPixelToOS(point.y()));
    }

    QHash<WId, QRiscosBackingStore *> m_surfaceHash;

    osspriteop_trans_tab const *translationTable() const {
	return mTranslationTable;
    }

private:
    void generateTranslationTable();

private:
    QRect mGeometry;
    int mDepth;
    QImage::Format mFormat;
    int mXEigenFactor;
    int mYEigenFactor;
    QRiscosCursor *mCursor;

    // The translation table to use when plotting a 32bit sprite to this screen.
    // NULL if the screen is also 32bit.
    osspriteop_trans_tab *mTranslationTable;
};

QT_END_NAMESPACE

#endif
