#ifdef D
#else
#define D 10
#endif

#ifdef _N
#else
#define _N 10
#endif

typedef struct _ReviewOnItem
{
	int vid;
	float rating;
	short count;
	struct _ReviewOnItem* next;
	/* data */
} REVIEW_ON_ITEM, *PREVIEW_ON_ITEM;

void debugRatingMatrix(PREVIEW_ON_ITEM* rm);
PREVIEW_ON_ITEM* initialRatingMatrix(char* filename, PREVIEW_ON_ITEM* rating_matrix);
void initialFeatureVector(double (*featureV)[D], int num);
void initialBiasVector(double* biasV, int num);