// Still need to add:
//  -support for red (currently only works for blue)
//  -support for bottom side (currently only works on top)
//  -better picture taking
//  -also needs to test against other programs
//  -bring back spyLoc from the comments

//Declare any variables shared between functions here
int targetPOI;
float zero[3];
float earth[3];
float darkzone[3];
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
	darkzone[0]=0.35;
	darkzone[1]=0.0;
	darkzone[2]=0.0;

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
    	float poiLoc[3];
    	game.getPOILoc(poiLoc, targetPOI);
	//This function is called once per second.  Use it to control the satellite.
	if (game.getNextFlare() < 25 && game.getNextFlare() !=-1)
	{
	    arcMove(darkzone);
        if (game.getMemoryFilled()>0) {
            upload();
        }

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
	else if (elapsed>215 && game.getMemoryFilled()>0) {
        upload();
    }
	else
	{
    	//float target[3] = {0.0, 0.0, color*0.49};
    	//api.setPositionTarget(target);
        if (game.getMemoryFilled()==game.getMemorySize()) 
        { //If SPHERE's memory is full
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
	float angleDiff = angleBetween(myPos, target);
	if (angleDiff < 0.6) { 
	// It shouldn't track the angle the whole time because it's too slow 
	//   and won't catch up to the POI as it approaches
	    mathVecSubtract(attTarget,target,myPos,3); // point at target
	}
	else {
	    mathVecSubtract(attTarget,zero,myPos,3); // point at origin
	}
	mathVecNormalize(attTarget,3);
	api.setAttitudeTarget(attTarget);
	
	//Go to point outwards from target
	float shootFrom[3] = {0.0, (0.51/0.2)*target[1], color*0.51}; // myPos, but on the target's z-coordinate
	arcMove(shootFrom);
	
    if (distanceVec(myPos,target) < 1.50) {
        
        // these next 3 lines are all made for debugging and aren't actually needed
        float myAtt[3] = {myState[6],myState[7],myState[8]};
        float angleDiff2 = angleBetween(myAtt,attTarget);
        DEBUG((" posAngleDiff = %f , attAngleDiff = %f", angleDiff, angleDiff2));
        
        if ((game.alignLine(targetPOI)==true) && (angleDiff < 0.4)) { 
            game.takePic(targetPOI);
        }
    }
}

float findMin(float a, float b, float c)
{ // returns the number term of the smallest one (0,1,2)
    if (a <= b && a <= c)
    {
        return 0;
    }
    else if (b <= a && b <= c)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

int findClosestPOI()
{
    float poi0[3];
    float poi1[3];
    float poi2[3];
    game.getPOILoc(poi0, 0);
	game.getPOILoc(poi1, 1);
	game.getPOILoc(poi2, 2);
    
    //DEBUG(("POI Y values: %f, %f, %f, %i | ", poi0[1],poi1[1],poi2[1],findMin(poi0[1],poi1[1],poi2[1])));
    
    if (poi0[1] > 0.11 || poi1[1] > 0.11 || poi2[1] > 0.11) {
        // if the side ones are too far to the side, it's not worth getting them
        return findMin(fabsf(poi0[1]),fabsf(poi1[1]),fabsf(poi2[1]));
        // ^ picks the middle one
    }
    else {
    	float aAngle = atan2(-poi0[2],poi0[0]); 
    	// Angle in the XZ plane, starts at -pi (I think) at bottom, goes to pi at the top
    	//   then resets to -pi as it crosses the dark zone
    	//   ^actually it might be pi/2
    	float bAngle = atan2(-poi1[2],poi1[0]);
    	float cAngle = atan2(-poi2[2],poi2[0]);
    	
    	return findMin(-aAngle,-bAngle,-cAngle); 
    	// finds the maximum angle, or the one that's going to rotate to the top next
    } 
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
        }
        if (oldScore >= game.getScore())
        {
            //DEBUG(("Upload failed!"));
        }
    }
}

void arcMove(float posTarget2[3])
{
    float midpoint[3] = {(myPos[0]+posTarget2[0])/2, (myPos[1]+posTarget2[1])/2, (myPos[2]+posTarget2[2])/2};
    if (mathVecMagnitude(midpoint,3) < 0.35) {
        mathVecNormalize(midpoint,3);
     	for (int i = 0; i<3; i++) {
    	 	midpoint[i] *= 0.49;
     	}
     	api.setPositionTarget(midpoint);
     	//DEBUG((" | Heading to waypoint | "));
    }
    else {
        api.setPositionTarget(posTarget2);
    }
}

float distanceVec(float a[3], float b[3]) {  //finds distance between two objects
	float diff[3];
	mathVecSubtract(diff,a,b,3);
	return mathVecMagnitude(diff,3);
}

float angleBetween(float a[3], float b[3]) {
    float aMag = mathVecMagnitude(a,3);
    float bMag = mathVecMagnitude(b,3);
    float dotProduct = (a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
    return acosf(dotProduct/(aMag*bMag));
}
