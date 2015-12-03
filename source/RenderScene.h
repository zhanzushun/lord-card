
#pragma once

#include "Config.h"

class GRenderScene
{
public:	
	static void Render(
		bool bDrawCard, 
		bool bDrawBottomCards = false, 
		bool bDisplayBottomCards = false, 
		bool bDrawCenterCard = false);

	static void RenderMovingCard(
		bool bDrawBottomCards, 
		bool bDisplayBottomCards, 
		bool bDrawCenterCard,
		Card* pMovingCard, CPoint p1, CPoint p2, int nFrames, bool bBack);
};