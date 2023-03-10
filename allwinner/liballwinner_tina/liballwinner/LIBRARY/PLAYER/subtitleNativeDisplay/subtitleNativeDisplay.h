/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CEDARX_SUBRENDER_H
#define _CEDARX_SUBRENDER_H

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <SkCanvas.h>
#include <SkBitmap.h>
#include <utils/VectorImpl.h>
#include <utils/Vector.h>
#include "subtitleNativeDisplayApi.h"
using namespace std;

#define  MAX_SUBLENGTH				500
#define  TEXT_SUBTITLE				1
#define  PIC_SUBTITLE				2
#define  BOTH_SUBTITLE				3

namespace android
{
	class CedarXSubTextBox
	{
		public:
		    CedarXSubTextBox();

		    enum CedarXSubTextBoxMode
		    {
		        CedarXSubTextBoxOneLine_Mode,
		        CedarXSubTextBoxLineBreak_Mode,

		        CedarXSubTextBoxModeCount
		    };

		    CedarXSubTextBoxMode    getMode() const { return (CedarXSubTextBoxMode)fMode; }
		    void    setMode(CedarXSubTextBoxMode);

		    enum CedarXSubTextBoxSpacingAlign
		    {
		        CedarXSubTextBoxStart_SpacingAlign,
		        CedarXSubTextBoxCenter_SpacingAlign,
		        CedarXSubTextBoxEnd_SpacingAlign,

		        CedarXSubTextBoxSpacingAlignCount
		    };
		    CedarXSubTextBoxSpacingAlign    getSpacingAlign() const { return (CedarXSubTextBoxSpacingAlign)fSpacingAlign; }
		    void            setSpacingAlign(CedarXSubTextBoxSpacingAlign);

		    void    subGetBox(SkRect* pRect) const;
		    void    subSetBox(const SkRect& mRect);
		    void    subSetBox(SkScalar mLeft, SkScalar mTop, SkScalar mRight, SkScalar mBottom);

		    void    subGetSpacing(SkScalar* pMul, SkScalar* pAdd) const;
		    void    subSetSpacing(SkScalar mMul, SkScalar mAdd);

		    void    drawText(SkCanvas*, const char text[], size_t len, const SkPaint&, SkScalar textHeight, int specialEffectFlag);
		    int		countLines(const char* text, size_t len, const SkPaint&, SkScalar width,  SkScalar* subTextWidth, int specialEffectFlag);
            int     getLastXPos();
            int     getLastYPos();
            int     getTextVerInf(const char *text, size_t len, const SkPaint& paint,SkScalar* subTextHeight,SkScalar* subTextWidth,SkScalar* textBoxHeight, int textBoxStartx,
                                         int textBoxEndx, int textBoxStarty, int textBoxEndy,int specialEffectFlag);

		private:
		    SkRect      mFBox;
		    SkScalar    mFSpacingMul, mFSpacingAdd;
		    uint8_t     fMode, fSpacingAlign;
            int         mLastDispXPos;
            int         mLastDispYPos;
	};

	class CedarXSub : public RefBase
	{
		public:
			CedarXSub(int index, int userSetFontColor, int userSetFontSize, int userSetFontStyle, int userSetYpercent);
			~CedarXSub();
			int					setPosition(int posX,int posY);
			int					setPositionYPercent(int percent);
			int				setTextColor(int color);
			int				getTextColor();
			int				getPositionX();
			int				getPositionY();
			int				setFontSize(int fontSize);
			int				getFontSize();
			int				setCharset(int Charset);
			int				getCharset();
			int				getWidth();
			int				getHeight();
			int				updatePara(sub_item_inf *sub_info, int initSetFontColor, int initSetFontSize, int initSetFontStyle, int Ypercent);
			int				Show();
			int				Hide();
			int					setBackColor(int color);
			int					getBackColor();
			int					setLayer(int layer);
			int					getLayer();
			int					setTextAlign(int align);
			int					getTextAlign();
			int					setZorderBottom();
			int					setZorderTop();
			int					setFontStyle(int style);
			int				getFontStyle();
	    int			setTextBox(sub_item_inf *sub_info,int startx, int starty, int endx, int endy, int lastDispx, int lastDispy, int newShowSubFlag, int yPercent);
            int                 getTextBox(int* startx, int *starty, int* endx, int* endy, int* lastDispx, int* lastDispy);
            int                 setSubInf(sub_item_inf *sub_info,int startx, int starty, int endx, int endy, int lastDispx, int lastDispy, int newSubShowFlag, int yPercent);
            int                 setSubShowFlag(int subShowFlag);
            int                 getSubShowFlag();
            int                 getTextColorFlag();
            int                 getTextFontSizeFlag();
            int                 getTextFontStyleFlag();
            int                 needModifyBoxInf(SkScalar textBoxHeight);
            unsigned int        getSubShowEndTime();
            int                 processSpecialEffect(unsigned int systemTime);
            bool				mShow;
		private:
			int					startRender();
			int					endRender();
			int					render();
			int					getBitmapFormat();
			int					convertUniCode(sub_item_inf *sub_info);
			int					generateBitmap(sub_item_inf *sub_info);
			int					mapDecToRender(int deccharset);
            int                 startRenderRegion(int startx, int starty, int endx, int endy);
			int				mCharset;
			int				mFontSize;
			int					mSaveCount;
			unsigned int        mTextColor;
			int					mTextAlign;
			int					mSubMode;
			SkCanvas*		mCanvas;
			CedarXSubTextBox*	mTextBox;
			sp<SurfaceControl>	mSurfaceControl;
			sp<SurfaceComposerClient> mSurfaceClient;
			SkTypeface*			mTypeface;
			SkPaint				mPaint;
			int					mPosX;
			int					mPosY;
			bool				useText;
			bool				useBitmap;
			char				mText[MAX_SUBLENGTH];
			SkBitmap			mBitmap;
			int					mLayer;
			int					mTopBaseLayer;
			int					mBottomBaseLayer;
			int					mMaxWidth;
			int				mMaxHeight;
			int					mWidth;
			int					mHeight;
			int					mDisplay;
			int					mPid;

			int					mBackColor;
			int					mFontStyle;
			int					mScreenWidth;
			int					mScreenHeight;
			int					mFontScaleRatio;
			int					mMaxFontHeight;
			int					mMaxTextLine;
            int                 mStartx;
            int                 mEndx;
            int                 mStarty;
            int                 mEndy;
            int                 mStartDispx;
            int                 mStartDispy;
            int                 mSubShowFlag;
            int                 mAlignment;
            SkScalar            mTextHeight;
            SkScalar            mTextWidth;

       #if 0
            unsigned int        mSubShowEndTime;
            unsigned int        mSubShowStartTime;
            unsigned int        mSubEffectDelay;
            unsigned int        mSubEffectColor;
            int                 mSubSpecialEffect;
            int                 mSubEffectStartxPos;
            int                 mSubEffectEndxPos;
            int                 mSubEffectStartyPos;
            int                 mSubEffectEndyPos;

            int                 mSubHasColorFlag;
            int                 mSubHasFontSizeFlag;
            int                 mSubHasStyleFlag;
            int                 mSubScalerWidth;
            int                 mSubScalerHeight;
       #endif
           sub_item_inf *      mDispSubInfo;
	};

    class CedarXSubRender
    {
        public:
		CedarXSubRender();
		~CedarXSubRender();

			int		cedarxSubShow();
			int		cedarxSubHide(unsigned int systemTime, unsigned int* hasSubShowFlag);
			int		cedarxSubSetPosition(int index,int positionx,int positiony);
			int		cedarxSubSetYPercent(int index,int percent);
			int		cedarxSubGetPositionX(int index);
			int			cedarxSubGetPositionY(int index);
			int		cedarxSubSetFontColor(int color);
			int		cedarxSubGetFontColor();
			int			cedarxSubSetBackColor(int color);
			int			cedarxSubGetBackColor();
			int		cedarxSubSetFontSize(int size);
			int		cedarxSubGetFontSize();
			int		cedarxSubSetCharset(int charset);
			int		cedarxSubGetCharset();
			int		cedarxSubGetHeight(int index);
			int		cedarxSubGetWidth(int index);
			int		cedarxSubSetDisplay(int display);
			int		cedarxSubGetDisplay();
			int			cedarxSubSetZorderBottom();
			int			cedarxSubSetZorderTop();
			int			cedarxSubSetAlign(int align);
			int			cedarxSubGetAlign();
			int			getCurrentSubNum();
			int			cedarxSubSetFontStyle(int style);
			int         cedarxSubGetFontStyle();
			int			updateSubPara(sub_item_inf *sub_info);
        private:
            int							mDisplay;
            int							mPid;
            Vector<sp<CedarXSub> >		mCedarXSubs;
            int							mBaseLayer;
            int                mUserSetFontColor;
            int                mUserSetFontSize;
            int                mUserSetFontStyle;
            int                mInitSetFontColor;
            int                mInitSetFontSize;
            int                mInitSetFontStyle;
            int                mUserSetYPercent;
            sub_item_inf       *sub_pre;

    };
} // namespace android

#endif // _UI_INPUT_DISPATCHER_H
