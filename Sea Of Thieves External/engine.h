#pragma once
#include "precomp.h"







bool updateWorld();

std::string GetNameFromFName(int key);



bool updateLocalPlayer();




FMinimalViewInfo getViewInfo();




bool refreshPawns();


void drawSkeleton(AFortPlayerState player);


void drawBoneIDs(AFortPlayerState player);


ImColor getColorByRarity(INT8 rarity);


void drawDebugAActors();





void drawWorldItems();
std::string getPlayerName(uintptr_t pieceChar);

void projectileTP();


void mainLoop();

















