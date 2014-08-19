#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include "prediction.h"

#define DEBUG_PREDICTION_PRINT1 
#define DEBUG_PREDICTION_PRINT2 printf
#define DEBUG_PREDICTION_PRINT3 
#define DEBUG_PREDICTION_PRINT4 

void Euclidean(struct Distnode *ptr1, double** distance, int n, double coord_gridX, double coord_gridY);
// double Determinate(double **a,int n);
void gauss_Pivot(double **a, int n);
void LUdecomposition(double **a, double *b ,int n, double *x);
double sphericalModel (double range, double sill, double h);
void LU_de_Pivot(double **u, double **l, double *b, int n);
void U_copy(double **u, double **a, int n);
void Print_1(double *a, int n);
void Print_2(double **a, int n);
void chan(double **a, double *b, double *y, int n);
void u_chan(double **a, double *y, double *x, int n);

int prediction(struct Distnode *Head_distfromGrid, int numofNearestPoint, double range, double sill, int numofPoints, int gridX, int gridY, double coord_gridX, double coord_gridY, double** distance){
	
	//const int N = numofNearestPoint+1;
	//const int GridPoints = 10000;
	const int N = numofPoints+1;

	//double range, sill; 
	FILE *semiParam;

	struct Distnode *ptr1, *ptr2;

	int i, j, k, n;
	//double **distance, 
	double **gamma;
	char ch;
	FILE *fGridPoints;

	double *x, *y, *z; // Read to data
	// char line[2000];
	// char lidarxyz[100];
	int count_row=0, wgt_idx;
	double predict=0;
	FILE *fGridPrediction;


	/*semiParam = fopen ("SemivariogramModelParameter.txt", "r");
	if (semiParam==NULL){
		printf ("cannot open semivariogram parameter file");
		return 0;
	}
	fscanf (semiParam, "%lf %lf", &range, &sill);
	printf ("range : %lf, sill : %lf\n", range, sill);
	fclose(semiParam);*/

	/*fGridPoints = fopen("PointsinGrid.txt","r");
	if(fGridPoints==NULL){
		printf("Not exist file");
		return 0;
	}*/
	
	//file open to write prediction value per grid point
	fGridPrediction = fopen("PredictionperGridpoint.txt","a");
	if(fGridPrediction==NULL){
		printf("it doesnt make file");
		return 0;
	}

	/*============= Input to varVal from Valrigoram fit ============*/

	//distance all points
	//distance = (double**)malloc(sizeof(double*)*N);
	
	//known and unknown value, left and right side
	//it is finally changed to storing weight value
	gamma = (double**)malloc(sizeof(double*)*N);
	
	// for(i=0;i<N;i++) {
	// 	distance[i] = (double*)malloc(sizeof(double)*N);
	// }
	for(i=0;i<N;i++) {
		gamma[i] = (double*)malloc(sizeof(double)*(N+1));
	}

		/*================== Read Lidar Data =================*/
/*	x = (double*)malloc(sizeof(double)*(N));
	y = (double*)malloc(sizeof(double)*(N));
	z = (double*)malloc(sizeof(double)*(N));
*/
	//initialize distance
	// for (i=0 ; i<N ; i++){
	// 	for (j=0 ;j<N ; j++){
	// 		distance[i][j] =0.0;
	// 	}
	// }

	ptr1 = Head_distfromGrid;

	//printf ("sill %lf , range %lf\n", sill, range);
	//printf ("before cal dist\n");
	//----------- calculate distance
	Euclidean(ptr1, distance, N, coord_gridX, coord_gridY);

	// double **a = (double **)malloc(sizeof(double*)*(N));
	// double **l = (double **)malloc(sizeof(double*)*(N));
	// double **u = (double **)malloc(sizeof(double*)*(N));
	// for(i=0;i<N;i++) {
	// 	a[i] = (double *)malloc(sizeof(double)*(N));
	// 	l[i] = (double *)calloc(N, sizeof(double));
	// 	u[i] = (double *)calloc(N, sizeof(double));
	// }
	// for (i=0 ; i<N ; i++){
	// 	l[i][i]=1.0;
	// }

	// double *b = (double *)malloc(sizeof(double)*(N));
	// double *itm_z = (double *)malloc(sizeof(double)*(N));
	// double *sol = (double *)malloc(sizeof(double)*(N));

	//printf ("before cal gamma\n");
	//----------- Applied Gamma Function for Ordinary Kriging
	for(i=0;i<N;i++){
		for(j=i;j<N;j++){
			if(j==N-1) {
				gamma[i][j] = -1.0;
				//a[i][j] = gamma[i][j];
				//u[i][j] = gamma[i][j];

				if (distance [i][j] < range){
					gamma[i][N] = sphericalModel(range, sill, distance[i][j]);
					//b[i] = gamma[i][N];
				}
				else{
					gamma[i][N] = sill;
					//b[i] = gamma[i][N];
				}
			}
			else{
				if (distance [i][j] < range){
					gamma[i][j] = sphericalModel(range, sill, distance[i][j]);
					//a[i][j] = gamma[i][j];
					//u[i][j] = gamma[i][j];
				}
				else{
					gamma[i][j] = sill;
					//a[i][j] = gamma[i][j];
					//u[i][j] = gamma[i][j];
				}
				gamma[j][i] = gamma[i][j];
				//a[j][i] = a[i][j];
				//u[j][i] = u[i][j];
				gamma[N-1][j] = 1.0;
				//a[N-1][j]=1.0;
				//u[N-1][j]=1.0;
			}
		}
	} 
	gamma[N-1][N-1] = 0.0;
	//a[N-1][N-1] = 0.0;
	//u[N-1][N-1] = 0.0;
	gamma[N-1][N] = 1.0;
	//b[N-1] = 1.0;
	
	
	/*//for debugging
		DEBUG_PREDICTION_PRINT2 ("N : %d\n", N);
		for(i=0;i<N;i++){
			DEBUG_PREDICTION_PRINT2("D");
			for(j=0;j<N;j++){
				DEBUG_PREDICTION_PRINT2("%10.4lf", distance[i][j]);
			}
			DEBUG_PREDICTION_PRINT2("\n");
		}

	//for debugging
		for(i=0;i<N;i++){
			DEBUG_PREDICTION_PRINT2("G");
			for(j=0;j<N+1;j++)
				DEBUG_PREDICTION_PRINT2("%10.4lf", gamma[i][j]);
			DEBUG_PREDICTION_PRINT2("\n");
		}
		*/

		//printf ("before cal gauss_elimination\n");
	
	//------solve linear equation to get weight
		//printf ("before malloc\n");


		
		// printf ("a\n");
		// for (i=0 ; i<N ; i++){
		// 	for (j=0 ; j<N ; j++){
		// 		//a[i][j] = gamma[i][j];
		// 		printf ("%lf, ", a[i][j]);
		// 	}
		// 	printf ("\n");
		// }

		// printf ("b\n");
		// for  (i=0 ; i<N ; i++){
		// 	//b[i] = gamma[i][N];
		// 	printf ("%lf, ", b[i]);
		// }
		// printf ("\n");

		
		//printf ("before lu\n");
		//LUdecomposition(a, b, N, sol);

		//U_copy(u, a, N);

		//LU decomposition equation solver
		// LU_de_Pivot(u, l, b, N);
		// chan(l, b, itm_z, N);
		// u_chan(u, itm_z, sol, N);


		//previous equation solver
		gauss_Pivot (gamma, N);
		//previous equation solver


		// for (i=0 ; i<N ; i++){
  //   			free(a[i]);
  //   			free(l[i]);
  //   			free(u[i]);
  //   		}
  //   		free(a);
  //   		free(b);
  //   		free(itm_z);
  //   		free(l);
  //   		free(u);

		//printf ("after cal gauss_elimination\n");
	//for debugging
		/*
		for(i=0;i<N;i++){
			DEBUG_PREDICTION_PRINT3("Weight\n");	
			DEBUG_PREDICTION_PRINT3("%10.4lf", gamma[i][N]);
			DEBUG_PREDICTION_PRINT3("\n");
		}
		DEBUG_PREDICTION_PRINT3 ("\n");
	*/

		double weightsum=0.0;
	//calculate predict value in grid or radius or nearest points 
		ptr1 = Head_distfromGrid;
		for (i=0 ; i<N-1 ; i++){
			predict += ptr1->coord->Z * gamma[i][N-1];
			weightsum += gamma[i][N-1];
			ptr1=ptr1->next;
		}

		//free(sol);

		//printf ("before write file\n");
		fprintf (fGridPrediction, "[%2.2lf][%2.2lf], %lf , nrPoints: %d, weightsum: %lf\n",coord_gridX, coord_gridY, predict, numofPoints, weightsum);

		predict = 0;
	

	//printf ("before free\n");
	// ---------------- Free variable
	for(i=0;i<N;i++) {
		free(distance[i]);
		free(gamma[i]);
	}
	free(distance); free(gamma); 
	//free(x);free(y);free(z);
	

	fclose(fGridPrediction);
	return 0;
}

double sphericalModel (double range, double sill, double h){
	return sill*pow( (3*h/(2*range)) - 1/2*(h/range) , 3);
}

void Euclidean(struct Distnode *ptr1, double** distance, int n, double coord_gridX, double coord_gridY) {
	//struct Distnode *ptr2;

	int i=0, j=0, N;
	double sumPow;
	N = n;

	while (ptr1){
		//printf ("%lf %lf %lf %lf\n",ptr1->coord->X, ptr2->coord->X,ptr1->coord->Y, ptr2->coord->Y  );
		distance[i][N-1] = sqrt (pow(ptr1->coord->X - coord_gridX,2)+ pow(ptr1->coord->Y - coord_gridY,2));
		//printf ("dist : %lf\n", ecd[i][j]);
		ptr1=ptr1->next;
		i++;
	}

}

void gauss_Pivot(double **a, int n) {
	int i,j,k,o,u,q,w,c;
	double temp1,temp2,temp3,max,num;
/*
	for (i=0 ; i<n ; i++){
		for (j=0 ; j<n ;j++){
			DEBUG_PREDICTION_PRINT1 ("%lf ", a[i][j]);
		}
	}
	DEBUG_PREDICTION_PRINT1("\nWith Pivot \n");
*/
	for(i=0;i<n-1;i++) {

  //pivot크기비교
		max=fabs(a[i][i]);
		for (q=i;q<n;q++) {
			num = fabs(a[q][i]);
			DEBUG_PREDICTION_PRINT4 ("num : %lf\n", num);
			if( num > max ) {
				DEBUG_PREDICTION_PRINT4("num>max : num: %lf\n", num);
				max = num;
				w = q;
			}
		}
  //더 큰 것이 있다면 변환
		DEBUG_PREDICTION_PRINT4 ("max : %lf, a[i][i] : %lf\n", max, a[i][i]);
		if(fabs(a[i][i]) != max) {
			for(c=0;c<n+1;c++) {
				temp3 = a[i][c];
				a[i][c] = a[w][c];
				a[w][c] = temp3;
			}

			// DEBUG_PREDICTION_PRINT4("Pivoting 출력 \n");
			// for(o=0; o< n; o++) {
			// 	for(u=0; u< n+1; u++ ) {
			// 		DEBUG_PREDICTION_PRINT4("%13.4lf\t", a[o][u]);
			// 	}
			// 	DEBUG_PREDICTION_PRINT4("\n");
			// }
		}

  //가우스소거법
		for(j=i+1; j<n; j++) {
			temp2 = a[j][i]/a[i][i];
			for(k=0 ; k<n+1; k++) {
				a[j][k] = a[j][k] - (temp2 * a[i][k]);
			}
		}

		// DEBUG_PREDICTION_PRINT4("과정 \n");
		// for(o=0; o< n; o++) {
		// 	for(u=0; u< n+1; u++ ) {
		// 		DEBUG_PREDICTION_PRINT4("%13.4lf\t", a[o][u]);
		// 	}
		// 	DEBUG_PREDICTION_PRINT4("\n");
		// }

	}

 //역변환
	for(i=n-1; i>=0; i--)
	{
		for(k=i+1; k<n; k++) {
			a[i][n]=a[i][n]-a[i][k]*a[k][n];
		}
		a[i][n]=a[i][n]/a[i][i];
	}

	DEBUG_PREDICTION_PRINT4("\n해 x는 \n");

	/*for(i=0; i<n; i++)
		DEBUG_PREDICTION_PRINT4("x%d의 해 : %13.4lf\t\n", i+1, a[i][n]);
*/
	return;
}


//LU소거법
void LU_de_Pivot(double **u, double **l, double *b, int n) {
  int i,j,k,p,q;
  double temp = 0 ,max;

  for(i= 0 ; i<n-1 ; i++) {
//pivot
    max = fabs(u[i][i]);
    for(p=i ; p<n ; p++) {
      if(fabs(u[p][i]) > max) {
        max = fabs(u[p][i]);
        q = p;
      }
    }
    //printf ("max : %lf \n", max);
    if(fabs(u[i][i]) != max) {
//U를 피봇
      for(p=0; p<n ; p++) {
        temp = u[i][p];
        u[i][p] = u[q][p];
        u[q][p] = temp;
      }

//L을 피봇
      for(p=0;p<i;p++){
        temp = l[i][p];
        l[i][p] = l[q][p];
        l[q][p] = temp;
      }

//B를 피봇
      temp = b[i];
      b[i] = b[q];
      b[q] = temp;
    }
    //printf ("u[i][i] : %lf\n", u[i][i]);
//소거법
    for(j=i+1 ; j<n ; j++) {
      if (u[i][i] == 0.0)
        temp = 0.0;
      else
        temp = u[j][i]/u[i][i];
      l[j][i] = temp;
      for(k=0 ; k<n; k++) {
        u[j][k] = u[j][k] - (u[i][k]*temp);
      }
    }
  }
}

//정치환 함수
void chan(double **a, double *b, double *y, int n) {
  int i,j;
  double sum;
  for(i=0 ; i<n ; i++) {
    sum =0;
    for(j=0;j<i;j++){
      sum += a[i][j]*y[j];
    }
    y[i] = (b[i]-sum)/a[i][i];
  }
}

//후치환 함수
void u_chan(double **a, double *y, double *x, int n) {
 int i,j;
 double sum;
 for(i=n-1 ; i>=0 ; i--) {
  sum =0;
  for(j=n-1 ; j>i ; j--){
    sum += a[i][j]*x[j];
   }
  x[i] = (y[i]-sum)/a[i][i];
 }
}


//U복사함수
void U_copy(double **u, double **a, int n) {
  int i,j;

  for(i=0 ; i<n ; i++) {
    for(j=0 ; j<n ; j++)
      u[i][j] = a[i][j];
  }
}

//출력함수
void Print_1(double *a, int n) {
  int i;
  for(i=0; i<n ; i++)
    printf("%9.4lf ", a[i]);
  printf("\n");
}
void Print_2(double **a, int n) {
  int i,j;
  for(i=0 ; i<n ; i++) {
    for(j=0 ; j<n ; j++) {
      printf("%9.4lf ", a[i][j]);
    }
    printf("\n");
  }
}