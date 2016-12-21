// Libraries
#include <stdio.h>
#include <stdlib.h>

// Classes
#include <ultrasonicInterface.h>
#include <errorHandler.h>

// Global variables
int currentDistances[];
int worstPath, bestPath;
int worstOption, bestOption;
int noSuitablePath;

/* Direction information
  0 = Forward
  1 = Right
  2 = Backward
  3 = Left
*/

/* Usage to determine best path
  1. sequentialRead
  2. pathEvaluation
  3. returnDecision
*/

void sequentialRead(){
  int i;
  for( i = 0; i < 4; i++ ){
    currentDistances[i] = getDistance(i);
  }

  return;
}

void pathEvaluation(){
  worstPath = 0;
  bestPath = 300;
  worstOption = 0;
  bestOption = 0;

  int i;
  for( i = 0; i < 4 ){
    if( currentDistances[i] < worstPath ){
      worstPath = currentDistances[i];
      worstOption = i;
    }
    if( currentDistances[i] > bestPath ){
      bestPath = currentDistances[i];
      bestOption = i;
    }
  }

  if( currentDistances[0] < 50 && currentDistances[1] < 50 && currentDistances[2] < 50 && currentDistances[3] < 50 ){
    throwError(100); // Select relevant error code from table
  }

  return;
}

int returnDecision(){
  if( noSuitablePath == 1 ){
    return(0);
  }

  return(bestOption);
}

int isDirectionSafe(int direction){
  int safe;

  sequentialRead();

  if( currentDistances[direction] > 120 ){
    safe = 1;
  }
  else{
    safe = 0;
  }

  return safe;
}
