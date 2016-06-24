#include <stdio.h>
#include <math.h>
#include "main.h"
#include "mf.h"

extern int usernum, itemnum;
extern int sw_bias, sw_reg;

extern double total_mean;
extern int total_count;

void matrixFactorization(double (*U)[D],double (*V)[D],double* bu, double*bv, PREVIEW_ON_ITEM* \
	rm_train,PREVIEW_ON_ITEM* rm_test, double* item_mean){
	double Alpha = 0.001;
	double Learn_loop=1000;
	double LambdaUV = 0.1;
	double LambdaBias = 0.01;
	double stop_condition=10;
	for (int learn_loop = 0; learn_loop < Learn_loop; ++learn_loop)
	{
		double cost = 0;
		double err = 0;
		for (int i = 1; i < usernum; ++i)
		{
			PREVIEW_ON_ITEM point_rating;
			point_rating = rm_train[i];
			while (point_rating)
			{
				int uid;
				int vid;
				uid = i;
				vid = point_rating->vid;
				// U(i)*V(j)
				double eval = 0;
				for (int d = 0; d < D; ++d)
				{
					eval += U[uid][d]*V[vid][d];
				}
				eval += total_mean;
				if (sw_bias)
				{
					eval += bu[uid]+bv[vid];
				}
				// U(i)*V(j)-R(i)(j)
				eval -= point_rating->rating;
				for (int d = 0; d < D; ++d)
				{
					U[uid][d] -= Alpha*eval*V[vid][d];
					V[vid][d] -= Alpha*eval*U[uid][d];
					if (sw_reg)
					{
						U[uid][d] -= LambdaUV*Alpha*U[uid][d];
						V[vid][d] -= LambdaUV*Alpha*V[vid][d];
					}
				}
				bu[uid] -= Alpha*eval;
				if (sw_reg)
				{
					bu[uid] -= Alpha*LambdaBias*bu[uid];
				}
				bv[vid] -= Alpha*eval;
				if (sw_reg)
				{
					bv[vid] -= Alpha*LambdaBias*bv[vid];
				}
				point_rating = point_rating->next;
			}
		}
		cost = rmse(U,V,bu,bv,item_mean,rm_train,rm_train);
		err = rmse(U,V,bu,bv,item_mean,rm_test,rm_train);
		if (err>stop_condition)
		{
			printf("Stop train\n");
			getchar();
			break;
		}
		stop_condition = err;
		printf("Iteration: %d \n", learn_loop);
		printf("Train rmse is: %f\n", cost);
		printf("Test rmse is: %f\n", err);
	}
}
double rmse(double(*U)[D],double(*V)[D],double*bu,double*bv,double*item_mean,PREVIEW_ON_ITEM* \
	rm_test,PREVIEW_ON_ITEM* rm_train)
{
	double mse=0;
	int mse_count=0;
	int debug_count=0;

	for (int i = 1; i < usernum; ++i)
	{
		PREVIEW_ON_ITEM point_rating;
		point_rating = rm_test[i];
		while (point_rating)
		{
			double eval = 0;
			if (rm_train[i] && item_mean[point_rating->vid]!=-1)
			{
				for (int d = 0; d < D; ++d)
				{
					eval+=U[i][d]*V[point_rating->vid][d];
				}
				eval+=total_mean+bu[i]+bv[point_rating->vid];
			}
			else if (!rm_train[i] && item_mean[point_rating->vid]!=-1)
			{
				eval = item_mean[point_rating->vid];				
			}
			else if (item_mean[point_rating->vid] == -1)
			{
				eval = total_mean;
				debug_count++;
			}

			mse+=pow(eval-point_rating->rating,2);
			mse_count++;
			point_rating = point_rating->next;
		}
	}
	return sqrt(mse/mse_count);
}
void makeRecommend(double(*U)[D],double(*V)[D],double*bu,double*bv,\
	PREVIEW_ON_ITEM*rm_train,double*item_mean,int(*recom_list)[_N],double(*recom_socre)[_N])
{
	for (int i = 1; i < usernum; ++i)
	{
		for (int j = 0; j < _N; ++j)
		{
			recom_list[i][j] = 0;
		}
	}
	for (int uid = 1; uid < usernum; ++uid)
	{
		printf("%d\n", uid);
		for (int vid = 1; vid < itemnum; ++vid)
		{
			PREVIEW_ON_ITEM point_rating;
			point_rating = rm_train[uid];
			int flag_pass = 0;
			while (point_rating)
			{
				if (point_rating->vid == vid)
				{
					flag_pass = 1;
					break;
				}
				point_rating = point_rating->next;
			}
			if (flag_pass)
			{
				continue;
			}
			double eval = 0;
			if (rm_train[uid] && item_mean[vid]!=-1)
			{
				for (int d = 0; d < D; ++d)
				{
					eval+=U[uid][d]*V[vid][d];
				}
				eval+=total_mean+bu[uid]+bv[vid];
			}
			else if (!rm_train[uid] && item_mean[vid]!=-1)
			{
				eval = item_mean[vid];
			}
			else if (item_mean[vid] == -1)
			{
				eval = total_mean;
			}
			
			for (int n = 0; n < _N; ++n)
			{
				if (eval>recom_socre[uid][n])
				{
					if (n==0)
					{
						recom_socre[uid][n] = eval;
						recom_list[uid][n] = vid;
					}
					else
					{
						recom_socre[uid][n-1] = recom_socre[uid][n];
						recom_list[uid][n-1] = recom_list[uid][n];
						recom_socre[uid][n] = eval;
						recom_list[uid][n] = vid;
					}
				}
			}
		}
	}
}
double Precision(int(*recom_list)[_N],PREVIEW_ON_ITEM* rm_test)
{
	int hit=0;
	for (int i = 1; i < usernum; ++i)
	{
		PREVIEW_ON_ITEM point_rating;
		point_rating = rm_test[i];
		while (point_rating)
		{
			if (point_rating->rating > 3)
			{
				for (int n = 0; n < _N; ++n)
				{
					if (recom_list[i][n] == point_rating->vid)
					{
						hit++;
						break;
					}
				}
			}
			point_rating = point_rating->next;
		}
	}

	return (double)hit/_N/usernum;
}