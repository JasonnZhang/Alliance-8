//Declare any variables shared between functions here
int targetPOI;
float zero[3];
float earth[3];
float darkzone[3];
int picturesTaken;
int color;
int elapsed;
float myPos[3];
ZRState myState;

void init(){
	//This function is called once when your code is first loaded.
	elapsed = 0;
    zero[0]=0.0;
	zero[1]=0.0;
	zero[2]=0.0;
	earth[0]=0.64;
	earth[1]=0.0;
	earth[2]=0.0;
	darkzone[0]=0.55;
	darkzone[1]=0.0;
	darkzone[2]=0.0;
	picturesTaken = 0;
	
	api.getMyZRState(myState);
	for (int i = 0; i<=2; i++) {
	    myPos[i] = myState[i];
	}
    if (myPos[1] > 0)
    {
        color = -1;
        DEBUG(("We are blue!")); //Determine if SPHERE is red or blue
    }
    else
    {
        color = -1;
        DEBUG(("We are red!"));
    }
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	
}

void loop(){
    api.getMyZRState(myState);
    for (int i = 0; i<=2; i++) {
	    myPos[i] = myState[i];
	}
    
    float poiSpyLoc[3];
    targetPOI = findClosestPOI();
    DEBUG(("POI: %d , ", targetPOI));
    	float poiLoc[3];
    	game.getPOILoc(poiLoc, targetPOI);
	//This function is called once per second.  Use it to control the satellite.
	if (game.getNextFlare() < 25 && game.getNextFlare() !=-1)
	{
	    api.setPositionTarget(darkzone);
        upload();

        game.getPOILoc(poiSpyLoc, targetPOI);
        /* if (poiSpyLoc[2]>0.15)
        {
            color = -1;
        }
        else
        {
            color = 1;
        } */
	}
	else if (elapsed>180 && game.getMemoryFilled()>0) {
        upload();
    }
	else
	{
    	float target[3] = {0.0, 0.0, color*0.44};
    	api.setPositionTarget(target);
    	
        if (game.getMemoryFilled()==game.getMemorySize()) 
        { //If SPHERE has a valid picture
            upload();
        }
        else {
            facePos(poiLoc);
        }

	}
	
	if (game.getFuelRemaining() <= 0)
    {
        game.turnOff();
        //DEBUG(("\nGame over man, game over!"));
    }
    
    elapsed++;	
}

void facePos(float target[3]){
	//Rotate to target
	float attTarget[3];
	mathVecSubtract(attTarget,target,myPos,3);
	mathVecNormalize(attTarget,3);
	api.setAttitudeTarget(attTarget);
	
	//Go to point outwards from target
	float shootFrom[3] = {0.0, (11/5)*target[1], color*0.44}; // myPos, but on the target's z-coordinate
	api.setPositionTarget(shootFrom);
	
    if (distanceVec(myPos,target) < 1.50)
    {
        //DEBUG(("The SPHERE is close enough to take a picture. ")); //
        float myPos2[3]; // = {myPos[0],myPos[1],myPos[2]};
        for (int i = 0; i<=2; i++) {
	        myPos[i] = myPos[i];
    	}
        mathVecNormalize(myPos2,3);
        
        float target2[3]; // = {target[0],target[1],target[2]};
        for (int i = 0; i<=2; i++) {
	        target2[i] = target[i];
    	}
        mathVecNormalize(target2,3);
        
        float angleDiff = distanceVec(myPos2,target2); // angle difference, in radians
        DEBUG(("angleDiff = %f", angleDiff));
        if (game.alignLine(targetPOI)==true) && (angleDiff < 0.4)) {
                //DEBUG(("\n  Pictures: %d", picturesTaken+1)); //      
                game.takePic(targetPOI);
                picturesTaken++;
        }
    }
	//return distance(attTarget, myPos);
}

float findMin(float a, float b)
{
    if (a < b)
    {
        return a;
    }
    else if (a == b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

int findClosestPOI()
{
    float poi0[3];
    float poi1[3];
    float poi2[3];
    
    //if (poi0[2] > 0.12) {
    //    return 1;
    //}
    //else {
    	game.getPOILoc(poi0, 0);
    	//float aDistance = distanceVec(poi0, myPos);
    	float aDistance = atan2(poi0[2],poi0[0]); // angle
    	game.getPOILoc(poi1, 1);
    	//float bDistance = distanceVec(poi1, myPos);
    	float bDistance = atan2(poi1[2],poi1[0]);
    	game.getPOILoc(poi2, 2);
    	//float cDistance = distanceVec(poi2, myPos);
    	float cDistance = atan2(poi2[2],poi2[0]);
    	
    	float closest = findMin(aDistance, findMin(bDistance, cDistance));
    	if (closest == aDistance)
    	{
    	    return 0;
    	}
    	else if (closest == bDistance)
    	{
    	    return 1;
    	}
    	else if (closest == cDistance)
    	{
    	    return 2;
    	}
    	else
    	{
    	    return -1;
    	    DEBUG(("\n   Targeting error has occurred :("));
    	}
    //}
}

void upload() {
    //float uploadTarget[3] = {0.1, 0.0, color*0.6};
    //api.setPositionTarget(uploadTarget);
    float attTarget[3];
	mathVecSubtract(attTarget,earth,myPos,3);
	mathVecNormalize(attTarget,3);
	api.setAttitudeTarget(attTarget);
    if (distanceVec(myPos, zero)> 0.0) //When SPHERE is out of both orbits, upload
    {
        float oldScore = game.getScore();
        
        game.uploadPic();
        if (oldScore < game.getScore())
        {
            DEBUG(("Upload was successful!"));
            picturesTaken = 0;
        }
        if (oldScore >= game.getScore())
        {
            DEBUG(("Upload failed!"));
        }
    }
}

float distanceVec(float a[3], float b[3]) {  //finds distance between two objects
	float diff[3];
	mathVecSubtract(diff,a,b,3);
	return mathVecMagnitude(diff,3);
}
