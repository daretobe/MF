#include <stdio.h>
#include <stdlib.h>
#include <sys/malloc.h>
#include <time.h>
#include "main.h"
#include "mf.h"

int usernum, itemnum;
int sw_bias, sw_reg;

double total_mean;
int total_count=0;

int main(int argc, char const *argv[])
{
	// some switch
	sw_bias = 1;
	sw_reg = 1;

	system("python choose.py");

	FILE *inputFile = fopen("epinions.stat","r");	
	fscanf(inputFile,"%d %d",&usernum,&itemnum);
	fclose(inputFile);
	// 把放在stack中的内容，改到heap中
	PREVIEW_ON_ITEM* rating_matrix_train;
	rating_matrix_train = malloc(sizeof(PREVIEW_ON_ITEM)*usernum);
	PREVIEW_ON_ITEM* rating_matrix_test;
	rating_matrix_test = malloc(sizeof(PREVIEW_ON_ITEM)*usernum);
	// initialize the rating matrix
	for (int i = 1; i < usernum; ++i)
	{
		rating_matrix_train[i] = NULL;
		rating_matrix_test[i] = NULL;
	} 
	PREVIEW_ON_ITEM* rm_train = initialRatingMatrix("epinions.train", rating_matrix_train);
	double item_mean[itemnum];
	short item_count[itemnum];
	for (int i = 1; i < itemnum; ++i)
	{
		item_mean[i] = 0;
		item_count[i] = 0;
	}
	for (int i = 1; i < usernum; ++i)
	{
		PREVIEW_ON_ITEM point_rating;
		point_rating = rm_train[i];
		while (point_rating)
		{
			point_rating->rating /= point_rating->count;
			// point_rating->rating -= total_mean;
			item_mean[point_rating->vid]+= point_rating->rating;
			item_count[point_rating->vid]++;
		
			point_rating = point_rating->next;
		}
	}
	printf("total mean:%f\n", total_mean);
	for (int i = 1; i < itemnum; ++i)
	{
		if (item_count[i]!=0)
		{
			item_mean[i]/=item_count[i];
		}
		else
		{
			item_mean[i]=-1;
		}
	}
	PREVIEW_ON_ITEM* rm_test = initialRatingMatrix("epinions.test", rating_matrix_test);
	double (*U)[D];
	double (*V)[D];
	double *bu;
	double *bv;

	U = malloc(sizeof(double)*D*usernum);
	V = malloc(sizeof(double)*D*itemnum);
	bu = malloc(sizeof(double)*usernum);
	bv = malloc(sizeof(double)*itemnum);

	// initial U, V, bu, bv
	initialFeatureVector(U,usernum);
	initialFeatureVector(V,itemnum);
	initialBiasVector(bu,usernum);
	initialBiasVector(bv,itemnum);

	printf("start mf here\n");
	getchar();

	matrixFactorization(U,V,bu,bv,rm_train,rm_test,item_mean);
	int (*recom_list)[_N];
	double (*recom_score)[_N];
	recom_list = malloc(sizeof(int)*_N*usernum);
	recom_score = malloc(sizeof(double)*_N*usernum);
	makeRecommend(U,V,bu,bv,rm_train,item_mean,recom_list,recom_score);

	double precision;
	precision = Precision(recom_list,rm_test);
	printf("precision:%f\n", precision);

	return 0;
}

void debugRatingMatrix(PREVIEW_ON_ITEM* rm)
{	
	// debug for filling rating matrix
	PREVIEW_ON_ITEM point_cursor;
	for (int i = 0; i < usernum; ++i)
	{
		for (int j = 0; j < itemnum; ++j)
		{
			point_cursor = rm[i];
			int flag_I = 0;
			while (point_cursor)
			{
				// printf("%d\n", point_cursor);
				if (point_cursor->vid == j)
				{	
					flag_I = 1;
					printf("%f ", point_cursor->rating);
					break;
				}
				point_cursor = point_cursor->next;
			}
			if (!flag_I)
			{
				printf("0 ");
			}
		}
		printf("\n");
	}
}

PREVIEW_ON_ITEM* initialRatingMatrix(char* filename, PREVIEW_ON_ITEM* rating_matrix)
{
	FILE *inputFile = fopen(filename,"r");
	int uid, vid;
	float rating;
	total_mean=0;
	total_count=0;

	// fill the rating matrix
	while (!feof(inputFile))
	{
		fscanf(inputFile,"%d %d %f",&uid,&vid,&rating);
		total_mean += rating;
		total_count++;
		if (uid == -1 && vid == -1 && rating == -1)
		{
			break;
		}
		if (rating_matrix[uid])
		{
			PREVIEW_ON_ITEM point_cursor;
			point_cursor = rating_matrix[uid];
			int flag_Dup = 0;
			do
			{
				if (point_cursor->vid == vid)
				{
					// printf("001 warning: user %d has rated item %d twice\n",uid,vid);
					// printf("previous rating:%d, now rating:%hd \n", point_cursor->rating,rating);
					point_cursor->count++;
					flag_Dup = 1;
					break;
				}
				if (point_cursor->next)
				{
					point_cursor = point_cursor->next;
				}
				else
				{
					break;
				}
			} while (1);
			if (!flag_Dup)
			{
				PREVIEW_ON_ITEM record = (PREVIEW_ON_ITEM)malloc(sizeof(REVIEW_ON_ITEM));
				record->next = NULL;
				record->vid = vid;
				record->rating = rating;
				record->count = 1;
				point_cursor->next = record;
			}
		}
		else
		{
			rating_matrix[uid] = (PREVIEW_ON_ITEM)malloc(sizeof(REVIEW_ON_ITEM));
			rating_matrix[uid]->next = NULL;
			rating_matrix[uid]->vid = vid;
			rating_matrix[uid]->rating = rating;
			rating_matrix[uid]->count = 1;
		}
	}
	fclose(inputFile);
	total_mean/=total_count;
	return rating_matrix;
}

void initialFeatureVector(double (*featureV)[D], int num)
{
	srand((unsigned)time(NULL));
	for (int i = 1; i < num; ++i)
	{
		for (int j = 0; j < D; ++j)
		{
			featureV[i][j] = (double)(rand()%1000000)/1000000;
		}
	}
}

void initialBiasVector(double* biasV, int num)
{
	srand((unsigned)time(NULL));
	for (int i = 1; i < num; ++i)
	{
		biasV[i] = (double)(rand()%1000000)/1000000;
	}
}
