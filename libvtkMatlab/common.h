#ifndef common_h
#define common_h

#include "mex.h"
#include "vtkDataArray.h"
#include "vtkPointSet.h"

void printVerbose(const char* msg, bool verbose);

int getNumberOfCellPoints(uint8_t cellType);

vtkDataArray* getDataArray(vtkPointSet* pointSet, char* dataAssociation, char* arrayName);

#endif /* common_h */
