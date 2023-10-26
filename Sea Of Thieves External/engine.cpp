#include "engine.h"

uintptr_t UWorld;
uintptr_t Levels;
INT32 levelCount;
uintptr_t persistentLevel;
uintptr_t GameInstance;
uintptr_t GameState;
uintptr_t localPlayers;
uintptr_t localPlayer;
uintptr_t localPlayerController;
uintptr_t localPlayerPiece;
int localPlayerID;

std::vector<AFortPawn> targetPawns;
std::vector<AFortPawn> enemyPawns;
std::vector<AActor> itemActors;


std::vector<FortItem> worldItems;

uintptr_t playerArray;
 
AimSettings currAimSetting;


AFortPlayerState localPlayerState;
int aimTarg = -1;
AFortPlayerState aimPiece;


ImVec2 boxDimensions;

FMinimalViewInfo viewInfo;

clock_t worldUpdateTimer = -4000;
clock_t localPlayerUpdateTimer = -4000;
clock_t pieceBaseUpdateTimer = -4000;
clock_t pieceUpdateTimer = -2000;
clock_t levelUpdateTimer = -4000;
clock_t itemUpdateTimer = -5000;


std::string GetNameFromFName(int key)
{


    constexpr auto ElementsPerChunk = 0x4000;

    enum { NAME_SIZE = 1024 };

    char name[NAME_SIZE] = { 0 };
    SecureZeroMemory(name, NAME_SIZE);


    const int32_t Index = key;



    uintptr_t fNamePtr1 = util::Read<uintptr_t>(GNames + 8 * (int)(Index / 0x4000) , sizeof(fNamePtr1));
    uintptr_t fNamePtr2 = util::Read<uintptr_t>(fNamePtr1 + 8 * (int)(Index % 0x4000), sizeof(fNamePtr2));



    const uint64_t AnsiName = fNamePtr2 + 0x10;


    int nameLength = NAME_SIZE - 1;
    ReadProcessMemory(hProc, reinterpret_cast<void*>(AnsiName), name, nameLength, NULL);
    std::string finalName = name;
    finalName = finalName.substr(0, nameLength);
    //delete[] name;

    return finalName;
}

bool updateWorld() {


    uintptr_t buffer = UWorld;

    clock_t tempTime = clock();
    if (tempTime >= worldUpdateTimer + 4000) {

        UWorld = util::Read<uintptr_t>(GWorld + modBase, sizeof(UWorld));


       

            Levels = util::Read<uintptr_t>(UWorld + offsets::UWorld::Levels, sizeof(Levels));

            levelCount = util::Read<INT32>(UWorld + offsets::UWorld::Levels + 8, sizeof(levelCount));

            persistentLevel = util::Read<uintptr_t>((UWorld + offsets::UWorld::PersistentLevel), sizeof(persistentLevel));

            GameInstance = util::Read<uintptr_t>((UWorld + offsets::UWorld::OwningGameInstance), sizeof(GameInstance));

            GameState = util::Read<uintptr_t>((UWorld + offsets::UWorld::GameState), sizeof(GameState));

            localPlayers = util::Read<uintptr_t>((GameInstance + 0x38), sizeof(localPlayers));

            localPlayer = util::Read<uintptr_t>(localPlayers, sizeof(localPlayer));

            localPlayerController = util::Read<uintptr_t>(localPlayer + 0x30, sizeof(localPlayerController));


            worldUpdateTimer = tempTime;

            return true;

       



    }
    return false;
}



bool updateLocalPlayer() {


    clock_t tempTime = clock();


    if (tempTime >= localPlayerUpdateTimer + 3000) {

     
        
        localPlayerController = util::Read<uintptr_t>(localPlayer + 0x30, sizeof(localPlayerController));

        localPlayerState = AFortPlayerState(util::Read<uintptr_t>((localPlayerController + 0x3e8), sizeof(uintptr_t)));

        localPlayerUpdateTimer = tempTime;
        return true;
    }
    return false;

}

std::string getPlayerName(uintptr_t pieceChar) {

    uintptr_t playerState = util::Read<uintptr_t>(pieceChar + 0x3e8, sizeof(playerState));
   

  
    int nameLength = util::Read<int>((playerState + 0x3e8), sizeof(nameLength));

   

    wchar_t* buffer = new wchar_t[nameLength * 2];
    SecureZeroMemory(buffer, nameLength * 2);

    uintptr_t nameAddr = util::Read<uintptr_t>((playerState + 0x3e0), sizeof(nameAddr));

    ReadProcessMemory(hProc, (BYTE*)(nameAddr), buffer, nameLength * 2, NULL);

    std::wstring ws(buffer);
    std::string str(ws.begin(), ws.end());
    delete[] buffer;

    return str;



}


FMinimalViewInfo getViewInfo() {

    

    FMinimalViewInfo currViewInfo;

        uintptr_t playerCamManager = util::Read<uintptr_t>(localPlayerController + 0x458, sizeof(playerCamManager));

        uintptr_t cameraCachePrivate = playerCamManager + 0x440;

        currViewInfo = util::Read<FMinimalViewInfo>(cameraCachePrivate + 0x10, sizeof(FMinimalViewInfo));

       

    





    viewInfo = currViewInfo;

    return viewInfo;

}



bool refreshPawns() {
    
    clock_t tempTime = clock();
    if (tempTime >= pieceUpdateTimer + 4000) {

        SecureZeroMemory(&targetPawns, sizeof(targetPawns));
        SecureZeroMemory(&enemyPawns, sizeof(enemyPawns));


        uintptr_t levelBase = util::Read<uintptr_t>(UWorld + offsets::UWorld::Levels, sizeof(levelBase));
        INT32 levelCount = util::Read<INT32>(UWorld + offsets::UWorld::Levels + 0x8, sizeof(levelCount));

        for (int level = 0; level < 1; level++) {


            uintptr_t currLevel = util::Read<uintptr_t>(levelBase + level * 8, sizeof(currLevel));

            uintptr_t  actorCluster = util::Read<uintptr_t>(currLevel + 0xa0, sizeof(actorCluster));

            INT32 actorClusterSize = util::Read<INT32>(currLevel + 0xa0 + 0x8, sizeof(actorClusterSize));

            for (int i = 0; i < actorClusterSize; i++) {

                AActor temp(util::Read<uintptr_t>(actorCluster + 0x8 * i, sizeof(uintptr_t)));

                if (!strcmp(temp.Name.c_str(), "BP_PlayerPirate_C")) {
                    AFortPawn tempPlayerPawn = AFortPawn(temp.baseAddress);

                    tempPlayerPawn.refreshSkeleton();

                    tempPlayerPawn.Name = getPlayerName(tempPlayerPawn.baseAddress);

                    targetPawns.push_back(tempPlayerPawn);


                }
                else if (!strcmp("BP_Mermaid_C", temp.Name.c_str())) {

                    AMermaid tempPlayerPawn = AMermaid(temp.baseAddress);

                    tempPlayerPawn.refreshSkeleton();

                    tempPlayerPawn.Name = "Mermaid";

                    targetPawns.push_back(tempPlayerPawn);
                }
                else if (strstr(temp.Name.c_str(), "Goldhoarder") != nullptr || strstr(temp.Name.c_str(), "Shopkeeper") != nullptr || strstr(temp.Name.c_str(), "Tavern") != nullptr || strstr(temp.Name.c_str(), "Sovereign") != nullptr || strstr(temp.Name.c_str(), "Castaway") != nullptr || strstr(temp.Name.c_str(), "Shipwright") != nullptr || strstr(temp.Name.c_str(), "Merchant") != nullptr) {

                    ANPC tempPlayerPawn = ANPC(temp.baseAddress);

                    tempPlayerPawn.refreshSkeleton();

                    tempPlayerPawn.Name = "NPC";

                    targetPawns.push_back(tempPlayerPawn);

                }
                else if (strstr(temp.Name.c_str(), "Chicken_") != nullptr) {


                    AFauna tempPlayerPawn = AFauna(temp.baseAddress);

                    tempPlayerPawn.refreshSkeleton();

                    uintptr_t NameBufPtr = util::Read<uintptr_t>(tempPlayerPawn.baseAddress + 0x818, sizeof(NameBufPtr));

                    int nameLength = util::Read<INT32>((NameBufPtr + 8), sizeof(nameLength));



                    wchar_t* buffer = new wchar_t[nameLength * 2];
                    SecureZeroMemory(buffer, nameLength * 2);

                    uintptr_t nameAddr = util::Read<uintptr_t>((NameBufPtr), sizeof(nameAddr));

                    ReadProcessMemory(hProc, (BYTE*)(nameAddr), buffer, nameLength * 2, NULL);

                    std::wstring ws(buffer);
                    std::string str(ws.begin(), ws.end());
                    delete[] buffer;

                    tempPlayerPawn.Name = str;

                    targetPawns.push_back(tempPlayerPawn);

                }
            }
            
        }


        enemyPawns.clear();
        enemyPawns.shrink_to_fit();

        for (int i = 0; i < targetPawns.size(); i++) {
            if (util::Read<uintptr_t>(targetPawns[i].baseAddress + 0x3e8, sizeof(uintptr_t)) != localPlayerState.baseAddress) {
                enemyPawns.push_back(targetPawns[i]);

            }


        }
      
            
       

        pieceUpdateTimer = tempTime;
        return true;
    }




    else {


        for (int i = 0; i < targetPawns.size(); i++) {




           targetPawns[i].refreshSkeleton();

            targetPawns[i].getDistance();

            targetPawns[i].getW2S();


           
                


        }



        enemyPawns.clear();
        enemyPawns.shrink_to_fit();
        for (int id = 0; id < targetPawns.size(); id++) {

            if (!targetPawns[id].skeleton[0][0].isZero() ) {


          
           
            
        }

        }

        for (int i = 0; i < targetPawns.size(); i++) {
            if (util::Read<uintptr_t>(targetPawns[i].baseAddress + 0x3e8, sizeof(uintptr_t)) != localPlayerState.baseAddress) {
                enemyPawns.push_back(targetPawns[i]);

            }


        }
    }

    return false;

}





void drawSkeleton(AFortPawn player) {


    BoneInfo previous(0, 0, 0, 0);
    BoneInfo current(0, 0, 0, 0);
    Vector3 p1, c1;
    for (int boneSet = 0; boneSet < player.skeleton.size(); boneSet++)
    {
        previous = BoneInfo(0, 0, 0, 0);
        for (int bone = 0; bone < player.skeleton[boneSet].size(); bone++)
        {
            current = player.skeleton[boneSet][bone];
            if (previous.x == 0.f)
            {
                previous = current;
                continue;
            }

            overlay::drawLine(Vector3(previous.W2S.x, previous.W2S.y, 0), Vector3(current.W2S.x, current.W2S.y, 0));
            previous = current;
        }
    }


}


void drawBoneIDs(AFortPawn player) {

    INT32 boneArrSize = util::Read<INT32>(player.meshAddress + 0x5d8+8, sizeof(boneArrSize));
    for (int i = 0; i < boneArrSize; i++) {

        FTransform bone = player.GetBoneIndex(i);

        FTransform ComponentToWorld = util::Read<FTransform>(player.meshAddress + 0x150, sizeof(FTransform));

        D3DMATRIX Matrix;
        Matrix = util::MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
        Vector3 boneWorld = Vector3(Matrix._41, Matrix._42, Matrix._43);

        overlay::drawText(util::WorldToScreen(boneWorld, viewInfo), 0.f, std::to_string(i).c_str());

    }


}


AFortPawn findPlayerByID(int32_t ID) {

    for (int i = 0; i < enemyPawns.size(); i++) {

      



    }
    SecureZeroMemory(&aimPiece, sizeof(aimPiece));
    return aimPiece.playerPawn;


}

ImColor getColorByRarity(INT8 rarity) {



    switch (rarity) {

    case(0): {

        return { 0.741f, 0.729f, 0.729f, 1.f };

    }
    case(1): {

        return { 0.039f, 0.769f, 0.f ,1.f };
    }
    case(2): {

        return { 0.02f, 0.529f, 0.769f ,1.f };
    }

    case(3): {

        return { 0.855f, 0.f, 0.922f , 1.f };
    }
    case(4): {

        return { 0.922f, 0.529f, 0.f , 1.f };
    }

           //chest
    case(5): {


        return{ 0.922f, 0.847f, 0.f , 1.f };
    }

           //ammoBox
    case(6): {

        return { 0.553f, 0.961f, 0.553f ,1.f };
    }


    }

}


void drawDebugAActors() {

    uintptr_t levelBase= util::Read<uintptr_t>(UWorld + offsets::UWorld::Levels, sizeof(levelBase));
    INT32 levelCount = util::Read<INT32>(UWorld + offsets::UWorld::Levels + 0x8, sizeof(levelCount));

    for (int level = 0; level < 3; level++) {


        uintptr_t currLevel = util::Read<uintptr_t>(levelBase + level * 8, sizeof(currLevel));

        uintptr_t  actorCluster = util::Read<uintptr_t>(currLevel + 0xa0, sizeof(actorCluster));

        INT32 actorClusterSize = util::Read<INT32>(currLevel + 0xa0 + 0x8, sizeof(actorClusterSize));

        for (int i = 0; i < actorClusterSize; i++) {




            AActor temp(util::Read<uintptr_t>(actorCluster + 0x8 * i, sizeof(uintptr_t)));
            temp.getDistance();
            temp.getW2S();

            overlay::drawText(temp.W2S, 0.f, temp.Name.c_str());

            char buffer[50];
            overlay::drawText(temp.W2S, 25.f, buffer);
            sprintf_s(buffer, "Base Addy:  0x%llX", temp.baseAddress);

            if (!strcmp("BP_SmallShipTemplate_C", temp.Name.c_str())) {

                ImGui::Text("%i Ship Found! %s @ 0x%llX", i, temp.Name.c_str(), temp.baseAddress);
            }
           
        }

   }
}


void updateItems() {
    
 


        std::vector<AActor> tempActors;

    clock_t tempTime = clock();

    if (tempTime >= itemUpdateTimer + 10000) {


        SecureZeroMemory(&itemActors, sizeof(itemActors));
        std::vector<AActor> tempActors;

        uintptr_t levelArr = util::Read<uintptr_t>(UWorld + offsets::UWorld::Levels, sizeof(levelArr));
        INT32 levelCount = util::Read<INT32>(UWorld + offsets::UWorld::Levels + 0x8, sizeof(levelCount));

        for (int level = 0; level < 1; level++) {

            uintptr_t levelBase = util::Read<uintptr_t>(levelArr + 0x8 * level, sizeof(levelBase));
            uintptr_t AActors = util::Read<uintptr_t>(levelBase + 0xa0, sizeof(AActors));
            INT32 AActorsSize = util::Read<INT32>(levelBase + 0xA8, sizeof(AActorsSize));
            for (int i = 0; i < AActorsSize; i++) {


                bool isOnList = false;


                AActor tempActor(util::Read<uintptr_t>(AActors + 0x8 * i, sizeof(uintptr_t)));




                if (!strcmp("BP_Ashenkey_Proxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Ashen key";

                }
                else if (!strcmp("BP_TreasureChest_Proxy_PirateLegend_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Athena Treasure Chest";
                }
                else if (!strcmp("BP_TreasureChest_Proxy_Mythical_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Captain Treasure Chest";
                }
                else if (!strcmp("BP_TreasureChest_Proxy_Fort_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Stronghold Treasure Chest";
                }
                else if (!strcmp("BP_LegendaryFort_StronholdKey_Proxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Legendary Stronghold Key";
                }
                else if (!strcmp("BP_MerchantCrate_BigGunPowderBarrelProxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Stronhold Gunpowder Barrel";
                }
                else if (!strcmp("BP_MerchantCrate_Commodity_Fort_Proxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Crate of Ancient Bone Dust";
                }
                else if (!strcmp("BP_MerchantCrate_Commodity_TeaCrate_Proxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Crate of Rare Tea";
                }
                else if (!strcmp("BP_BountyRewardSkull_Proxy_Fort_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Stronghold Skull";
                }
                else if (!strcmp("BP_MerchantCrate_CommonPirateLegend_Proxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Crate of Legendary Voyages";
                }
                else if (!strcmp("BP_MerchantCrate_Commodity_SugarCrate_Proxy_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Crate of Fine Sugar";
                }
                else if (!strcmp("BP_BountyRewardSkull_proxy_UncommonPirateLegend_C", tempActor.Name.c_str())) {

                    isOnList = true;
                    tempActor.Name = "Villainous Skull of Ancient Fortune";
                }
                else if (strstr(tempActor.Name.c_str(), "ShipTemplate") != nullptr) {

                    isOnList = true;
                    tempActor.Name = "Ship";
                }
            
               



                if (isOnList) {


                    tempActor.getPos();
                    tempActor.getW2S();
                    tempActor.getDistance();

                    tempActors.push_back(tempActor);




                }

            }

        }
        itemActors = tempActors;
        itemUpdateTimer = tempTime;
    }
    else {

        for (int i = 0; i < itemActors.size(); i++) {



           
            itemActors[i].getPos();
            itemActors[i].getW2S();
            itemActors[i].getDistance();
        


        }

    }
}

void drawWorldItems() {


    if (itemESP::enabled) {


        for (int i = 0; i < worldItems.size(); i++) {

            if (worldItems[i].Distance/100.f < itemESP::maxDistance) {

                if (itemESP::chests) {

                    if (worldItems[i].itemType == World_Chest) {

                        overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                    }

                }

                if (itemESP::ammoBox) {

                    if (worldItems[i].itemType == World_AmmoBox) {

                        overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                    }

                }

                if (itemESP::ammo) {

                    if (worldItems[i].itemType == Item_AMMO) {

                        overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());

                    }


                }
                if (itemESP::consumable) {

                    if (worldItems[i].itemType == Item_Consumable) {

                        overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                    }


                }
                if (itemESP::traps) {


                    if (worldItems[i].itemType == Item_Trap) {

                        overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                    }

                }
                if (itemESP::materials) {

                    if (worldItems[i].itemType == Item_Material) {

                        overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                    }

                }

                if (itemESP::weaponESP::enabled) {

                    if (itemESP::weaponESP::AR) {

                        if (worldItems[i].weaponType == WEAPON_AR) {

                            overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());



                        }

                    }
                    if (itemESP::weaponESP::SG) {

                        if (worldItems[i].weaponType == WEAPON_SG) {


                            overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                        }

                    }
                    if (itemESP::weaponESP::SMG) {

                        if (worldItems[i].weaponType == WEAPON_SMG) {


                            overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                        }

                    }
                    if (itemESP::weaponESP::Sniper) {

                        if (worldItems[i].weaponType == WEAPON_SNIPER) {


                            overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());


                        }

                    }
                    if (itemESP::weaponESP::Pistol) {

                        if (worldItems[i].weaponType == WEAPON_PISTOL) {

                            overlay::drawText(worldItems[i].W2S, 0.f, worldItems[i].drawColor, worldItems[i].itemName.c_str());



                        }

                    }




                }


            }

        }




    }




}


void mainLoop() {

    util::updateWindow();

    if (updateWorld()) {


    }

    if (UWorld && GameState) {

       
        getViewInfo();

        if (updateLocalPlayer()) {

        }


         if (refreshPawns()) {


          }
         updateItems();
         

         if (itemESP::enabled) {

             for (int i = 0; i < itemActors.size(); i++) {

                 overlay::drawTextSmall(itemActors[i].W2S, 0.f, itemActors[i].Name.c_str());

                 char buf[50];

                 sprintf_s(buf, "%i Meters\n", (int)itemActors[i].Distance / 100);

                 overlay::drawTextSmall(itemActors[i].W2S, 15.f, buf);

             }

         }
      
    }
    std::vector<AFortPawn> pawnsToLoop;

    if (bDrawFov) {

        ImGui::GetBackgroundDrawList()->AddCircle({ winProperties.width / 2 + winProperties.x, winProperties.height / 2 + winProperties.y }, currAimSetting.fovRadius, ImColor(1.f, 0.f, 0.f, 1.f), 100, 2.f);

    }

    if (localESP) {

        pawnsToLoop = targetPawns;

    }
    else {

        pawnsToLoop = enemyPawns;
    }
  

    if (playerIDESP) {
        drawDebugAActors();
    }

    for (int i = 0; i < pawnsToLoop.size(); i++) {


        if (pawnsToLoop[i].Distance > 0.f ) {

         //drawBoneIDs(pawnsToLoop[i]);



            boxDimensions = { (60 / (pawnsToLoop[i].Distance / 1000.f)) * (winProperties.width / 1920) , (110 / (pawnsToLoop[i].Distance / 1000.f)) * (winProperties.height / 1080) };

            if (strstr(pawnsToLoop[i].Name.c_str(), "Chicken") != nullptr) {

                boxDimensions = { ((60 / (pawnsToLoop[i].Distance / 1000.f)) * (winProperties.width / 1920))/2 ,((60 / (pawnsToLoop[i].Distance / 1000.f)) * (winProperties.width / 1920)) / 2 };

            }

            if (boxESP) {

                if (bRounded) {
                    overlay::drawBox(pawnsToLoop[i].W2S, rounding);

                }
                else {

                    overlay::drawBox(pawnsToLoop[i].W2S);


                }

            }

            if (lineESP) {


                overlay::drawLine(pawnsToLoop[i].W2S);

            }

            if (skeletonESP) {

                if (strcmp(pawnsToLoop[i].Name.c_str(), "NPC") )  
                {
                    drawSkeleton(pawnsToLoop[i]);
                }

            }

            if (distanceESP) {

                char distText[50];
                sprintf_s(distText, "%i Meters", ((int)pawnsToLoop[i].Distance / 100));

                if (strstr(pawnsToLoop[i].Name.c_str(), "Chicken") != nullptr) {
                    overlay::drawTextSmall(pawnsToLoop[i].W2S, boxDimensions.y + 15.f, distText);

                }
                else {
                    overlay::drawText(pawnsToLoop[i].W2S, boxDimensions.y + 15.f, distText);
                }
            }
         
            if (playerNameESP) {

                if (strstr(pawnsToLoop[i].Name.c_str(), "Chicken") != nullptr) {
                    overlay::drawTextSmall(pawnsToLoop[i].W2S, -boxDimensions.y - 10.f, pawnsToLoop[i].Name.c_str());

                }
                else {
                    overlay::drawText(pawnsToLoop[i].W2S, -boxDimensions.y - 10.f, pawnsToLoop[i].Name.c_str());
                }
            }



        }

    }

    if (drawTPS) {

        char TPSBuffer[50];
        sprintf_s(TPSBuffer, "TPS: %i", TPS);
        overlay::drawText(Vector3(winProperties.x + 40.f, winProperties.y + 20.f, 0), 0.f, TPSBuffer);

        char TotalBuf[50];
        sprintf_s(TotalBuf, "Average TPS %i", totalTPS / totalTime);
        overlay::drawText(Vector3(winProperties.x + 65.f, winProperties.y + 35.f, 0), 0.f, TotalBuf);

    }

   


}

















