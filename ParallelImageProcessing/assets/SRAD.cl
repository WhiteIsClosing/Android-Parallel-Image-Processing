
struct param{
	uint iWidth;
	uint iHeight;
	uint it;
	float mean;
	float q0;
	float3 empt;
};



__kernel void CQ(__constant struct param* par, __global  const float* imgSrc, __global float * cqM)
{

	int iWidth=par->iWidth;
	int iHeight=par->iHeight;
	int it=par->it;
	float mean=par->mean;
	float q0=par->q0;
	float pixelNum=(iWidth-4)*(iHeight-4);

	int iGIDx = get_global_id(0);
	int	iGIDy = get_global_id(1);
	int iGIDz = get_global_id(2);
	int iSIZEx = get_global_size(0);
	int iSIZEy = get_global_size(1);
	int iSIZEz = get_global_size(2);

	int idx= iGIDz*iSIZEx*iSIZEy + iGIDy * iSIZEx + iGIDx;
	

	int r = (idx/(iWidth-4))  + 2 ;
	int c = (idx%(iWidth-4))  + 2 ;
#define src(i,j) (imgSrc[ (i)*iWidth + (j) ])
#define des(i,j) (imgDes[ (i)*iWidth + (j) ])
#define cq(i,j)  (cqM[ (i)*iWidth + (j) ])

	
	if( idx < pixelNum ){ 
		float eps=0.0001;
		float a = ( -0.2 ) * ( it - 1);
		float qt = q0 * exp(a);
		
		float GradientX = ( src(r,c+1) - src(r,c-1) ) / 2;
		float GradientY = ( src(r+1,c) - src(r-1,c) )/ 2;
		float di1 = sqrt( GradientX * GradientX + GradientY * GradientY );
		float di2 = ( src(r,c+1) + src(r,c-1) + src(r+1,c) + src(r-1,c) )/4 - src(r,c);
#define SQUARE(t) ( ( t ) * ( t ) )
		float t1 = 0.5	* SQUARE( di1/(src(r,c) + eps) );
		float t2 = 0.625 * SQUARE( di2/(src(r,c) + eps) );
		float t3 = SQUARE( 1 + ( 0.25 * di2/(src(r,c) + eps) ) );
		float t  = sqrt( fabs( t1 - t2 ) / fabs( t3 + eps ) );
		float dd = ( t*t - qt*qt ) / ( qt*qt*(1+qt*qt)+eps );
			
		cq(r,c) = 1 / ( 1 + dd );
	}
}


__kernel void SRAD(__constant struct param* par, __global const float* cqM, __global const float* imgSrc, __global float* imgDes){
	int iWidth=par->iWidth;
	int iHeight=par->iHeight;
	int it=par->it;
	float mean=par->mean;
	float q0=par->q0;

	float pixelNum=(iWidth-4)*(iHeight-4);

	int iGIDx = get_global_id(0);
	int	iGIDy = get_global_id(1);
	int iGIDz = get_global_id(2);
	int iSIZEx = get_global_size(0);
	int iSIZEy = get_global_size(1);
	int iSIZEz = get_global_size(2);
	int idx= iGIDz*iSIZEx*iSIZEy + iGIDy * iSIZEx + iGIDx;

	int r = (idx/(iWidth-4))  + 2 ;
	int c = (idx%(iWidth-4))  + 2 ;
#define src(i,j) (imgSrc[ (i)*iWidth + (j) ])
#define des(i,j) (imgDes[ (i)*iWidth + (j) ])
#define cq(i,j)  (cqM[ (i)*iWidth + (j) ])
	
	if( idx < pixelNum ){	

		float GradientX_ = ( cq(r,c+1) * ( src(r,c+2) - src(r,c) ) - cq(r,c-1) * ( src(r,c) - src(r,c-2) ) )/4;
		float GradientY_ = ( cq(r+1,c) * ( src(r+2,c) - src(r,c) ) - cq(r-1,c) * ( src(r,c) - src(r-2,c) ) )/4;

	
		des(r,c) = src(r,c) + 0.035 * (GradientX_ + GradientY_);
		if(r==2 && c==2){
			des(2,2) = cq(r+1,c);
			des(2,3) = src(r+2,c);
			des(2,4) = src(r,c);
		}
	}
}



/*
cbuffer constt {
	uint iWidth;
	uint iHeight;
	uint it;
	float mean;
	float q0;
	float3 empt;
};
// group size
#define thread_group_size_x 512
#define thread_group_size_y 1

StructuredBuffer<float> imgSrc;
RWStructuredBuffer<float> cqM;

[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
void cqMain( uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, 
         uint groupIndex : SV_GroupIndex, 
         uint3 dispatchThreadID : SV_DispatchThreadID )
{
	float pixelNum = (iWidth-4)*(iHeight-4);
	int N_THREAD_GROUPS_X = ceil(pixelNum/(thread_group_size_x*thread_group_size_y)) ;
	int stride = thread_group_size_x * N_THREAD_GROUPS_X;  
	int idx = dispatchThreadID.y * stride + dispatchThreadID.x;
	int r = (idx/(iWidth-4))  + 2 ;
	int c = (idx%(iWidth-4))  + 2 ;
#define src(i,j) (imgSrc[ (i)*iWidth + (j) ])
#define des(i,j) (imgDes[ (i)*iWidth + (j) ])
#define cq(i,j)  (cqM[ (i)*iWidth + (j) ])

	
	if( idx < pixelNum ){ 
		float eps=0.0001;
		float a = ( -0.2 ) * ( it - 1);
		float qt = q0 * exp(a);
		
		float GradientX = ( src(r,c+1) - src(r,c-1) ) / 2;
		float GradientY = ( src(r+1,c) - src(r-1,c) )/ 2;
		float di1 = sqrt( GradientX * GradientX + GradientY * GradientY );
		float di2 = ( src(r,c+1) + src(r,c-1) + src(r+1,c) + src(r-1,c) )/4 - src(r,c);
#define SQUARE(t) ( ( t ) * ( t ) )
		float t1 = 0.5	* SQUARE( di1/(src(r,c) + eps) );
		float t2 = 0.625 * SQUARE( di2/(src(r,c) + eps) );
		float t3 = SQUARE( 1 + ( 0.25 * di2/(src(r,c) + eps) ) );
		float t  = sqrt( abs( t1 - t2 ) / abs( t3 + eps ) );
		float dd = ( t*t - qt*qt ) / ( qt*qt*(1+qt*qt)+eps );
			
		cq(r,c) = 1 / ( 1 + dd );
	}
}

*/

/*
cbuffer constt : register( b0 )
{
	uint iWidth;
	uint iHeight;
	uint it;
	float mean;
	float q0;
	float3 empt;
};
// group size
#define thread_group_size_x 512
#define thread_group_size_y 1

StructuredBuffer<float> cqM : register( t0 );
StructuredBuffer<float> imgSrc  : register( t1 );
RWStructuredBuffer<float> imgDes : register( u0 );


[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
void main( uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, 
         uint groupIndex : SV_GroupIndex, 
         uint3 dispatchThreadID : SV_DispatchThreadID )
{

	float pixelNum = (iWidth-4)*(iHeight-4);
	
	int N_THREAD_GROUPS_X = ceil(pixelNum/(thread_group_size_x*thread_group_size_y)) ;
	int stride = thread_group_size_x * N_THREAD_GROUPS_X;  
	int idx = dispatchThreadID.y * stride + dispatchThreadID.x;
	int r = (idx/(iWidth-4))  + 2 ;
	int c = (idx%(iWidth-4))  + 2 ;
#define src(i,j) (imgSrc[ (i)*iWidth + (j) ])
#define des(i,j) (imgDes[ (i)*iWidth + (j) ])
#define cq(i,j)  (cqM[ (i)*iWidth + (j) ])
	
	if( idx < pixelNum ){	

		float GradientX_ = ( cq(r,c+1) * ( src(r,c+2) - src(r,c) ) - cq(r,c-1) * ( src(r,c) - src(r,c-2) ) )/4;
		float GradientY_ = ( cq(r+1,c) * ( src(r+2,c) - src(r,c) ) - cq(r-1,c) * ( src(r,c) - src(r-2,c) ) )/4;

		des(r,c) = src(r,c) + 0.035 * (GradientX_ + GradientY_);
	}
	
}

*/