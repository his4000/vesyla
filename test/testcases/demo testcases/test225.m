
% Addition

% VESYLA code for normalizing in PCA made
% This code multiplies a matrix with sice A=M*N^2 with its transpose, where m is the mumber of columns and n^2 is the flattened image
% To get the dimensionality reduction we multiply (A)^t * A to get a final M*M matrix.

M = 8; % Total images
N = 16; % Image Dimension
IMAGE_SIZE = N*N; % Image size
MEM_BLOCK_SIZE = 16; % Number of words in a memory block
RFILE_SIZE = 32; % Size of a RegisterFile
invM = 1.0/M;

% Memory declarations
imageA_mem = fi([0.103601 0.003601 0.003662 0.003723 0.003815 0.003845 0.003540 0.003418 0.003479 0.003845 0.004120 0.004242 0.004303 0.004456 0.005096 0.003662 0.002014 0.001404 0.001831 0.002319 0.002106 0.001648 0.001831 0.001770 0.001617 0.001495 0.001404 0.001404 0.001465 0.001495 0.001831 0.001770 0.001740 0.001740 0.001801 0.001831 0.001801 0.001740 0.001556 0.001648 0.001801 0.001923 0.001923 0.001831 0.001678 0.001526 0.001343 0.001251 0.001343 0.001526 0.001526 0.001343 0.001251 0.001343 0.001740 0.001740 0.001740 0.001740 0.001740 0.001740 0.001740 0.001740 0.001465 0.001465 0.001495 0.001556 0.001617 0.001678 0.001740 0.001770 0.001648 0.001648 0.001678 0.001770 0.001801 0.001831 0.001923 0.001923 0.002075 0.002106 0.002167 0.002228 0.002319 0.002380 0.002472 0.002502 0.002747 0.002747 0.003052 0.002228 0.010010 0.009583 0.010010 0.009857 0.002563 0.002655 0.001801 0.002289 0.001923 0.002075 0.001953 0.001953 0.002014 0.002045 0.002106 0.002167 0.002197 0.002197 0.002228 0.002197 0.002167 0.002106 0.002075 0.002045 0.002014 0.001953 0.002106 0.002167 0.002167 0.002167 0.002197 0.002228 0.002228 0.002228 0.002441 0.002441 0.002472 0.002502 0.002563 0.002594 0.002625 0.002655 0.002655 0.002716 0.002747 0.002777 0.002869 0.002899 0.002930 0.002930 0.002869 0.002930 0.003113 0.003265 0.003326 0.003326 0.003296 0.003204 0.003448 0.003479 0.003479 0.003540 0.003571 0.003571 0.003601 0.003601 0.003693 0.003693 0.003693 0.003693 0.003693 0.003693 0.003693 0.003693 0.003845 0.003845 0.003815 0.003815 0.003754 0.003754 0.003723 0.003723 0.003662 0.003662 0.003662 0.003662 0.003662 0.003662 0.003662 0.003662 0.003448 0.003418 0.003387 0.003326 0.003326 0.003387 0.003448 0.003479 0.003448 0.003601 0.003723 0.003815 0.004120 0.004517 0.004639 0.004517 0.003937 0.002441 0.001221 0.001373 0.002045 0.002106 0.001740 0.001373 0.001801 0.001801 0.001770 0.001740 0.001740 0.001740 0.001740 0.001740 0.001404 0.001343 0.001251 0.001282 0.001373 0.001404 0.001343 0.001282 0.001251 0.001099 0.001007 0.001099 0.001343 0.001465 0.001343 0.001221 0.001495 0.001373 0.001373 0.001526 0.001526 0.001373 0.001373 0.001495 0.001404 0.001404 0.001404 0.001404 0.001404 0.001404 0.001404 0.001404 0.001495 0.001526 0.001556 0.001648 0.001740 0.001770 0.001801 0.001831 0.001801 0.001801 0.001831 0.001892], 1, 16, 15, 'RoundingMethod', 'Floor'); %! MEM<> [0,0]
imageB_mem = fi([0.103601 0.002014 0.001740 0.001343 0.001495 0.002167 0.001801 0.002075 0.002563 0.003326 0.003693 0.003662 0.004120 0.001740 0.001343 0.001404 0.003601 0.001404 0.001740 0.001526 0.001556 0.002228 0.002289 0.002045 0.002594 0.003326 0.003693 0.003662 0.004517 0.001740 0.001465 0.001404 0.003662 0.001831 0.001801 0.001526 0.001617 0.002319 0.001923 0.002014 0.002625 0.003296 0.003693 0.003662 0.004639 0.001740 0.001343 0.001404 0.003723 0.002319 0.001831 0.001343 0.001678 0.002380 0.002075 0.001953 0.002655 0.003204 0.003693 0.003662 0.004517 0.001740 0.001221 0.001404 0.003815 0.002106 0.001801 0.001251 0.001740 0.002472 0.001953 0.002106 0.002655 0.003448 0.003845 0.003448 0.003937 0.001404 0.001495 0.001495 0.003845 0.001648 0.001740 0.001343 0.001770 0.002502 0.001953 0.002167 0.002716 0.003479 0.003845 0.003418 0.002441 0.001343 0.001373 0.001526 0.003540 0.001831 0.001556 0.001740 0.001648 0.002747 0.002014 0.002167 0.002747 0.003479 0.003815 0.003387 0.001221 0.001251 0.001373 0.001556 0.003418 0.001770 0.001648 0.001740 0.001648 0.002747 0.002045 0.002167 0.002777 0.003540 0.003815 0.003326 0.001373 0.001282 0.001526 0.001648 0.003479 0.001617 0.001801 0.001740 0.001678 0.003052 0.002106 0.002197 0.002869 0.003571 0.003754 0.003326 0.002045 0.001373 0.001526 0.001740 0.003845 0.001495 0.001923 0.001740 0.001770 0.002228 0.002167 0.002228 0.002899 0.003571 0.003754 0.003387 0.002106 0.001404 0.001373 0.001770 0.004120 0.001404 0.001923 0.001740 0.001801 0.010010 0.002197 0.002228 0.002930 0.003601 0.003723 0.003448 0.001740 0.001343 0.001373 0.001801 0.004242 0.001404 0.001831 0.001740 0.001831 0.009583 0.002197 0.002228 0.002930 0.003601 0.003723 0.003479 0.001373 0.001282 0.001495 0.001831 0.004303 0.001465 0.001678 0.001740 0.001923 0.010010 0.002228 0.002441 0.002869 0.003693 0.003662 0.003448 0.001801 0.001251 0.001404 0.001801 0.004456 0.001495 0.001526 0.001740 0.001923 0.009857 0.002197 0.002441 0.002930 0.003693 0.003662 0.003601 0.001801 0.001099 0.001404 0.001801 0.005096 0.001831 0.001343 0.001465 0.002075 0.002563 0.002167 0.002472 0.003113 0.003693 0.003662 0.003723 0.001770 0.001007 0.001404 0.001831 0.003662 0.001770 0.001251 0.001465 0.002106 0.002655 0.002106 0.002502 0.003265 0.003693 0.003662 0.003815 0.001740 0.001099 0.001404 0.001892], 1, 16, 15, 'RoundingMethod', 'Floor'); %! MEM<> [0,1]
imageAvg_mem = fi(zeros(1,IMAGE_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! MEM<> [0,2]

% RegFile declarations
imageA   = fi(zeros(1,RFILE_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<input> [0,0]
imageB   = fi(zeros(1,RFILE_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<input> [0,1]
imageAvg = fi(zeros(1,RFILE_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<output> [0,2]
invM_vec = fi([invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM invM], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [1,0]

for i = 1 : M-1
	for j = 1 : IMAGE_SIZE/RFILE_SIZE
		imageA = imageA_mem((j-1)*RFILE_SIZE+1 : j*RFILE_SIZE);
		imageB = imageB_mem((j-1)*RFILE_SIZE+1 : j*RFILE_SIZE);
		imageB = imageA + imageB; %! DPU<sat> [0,0]
		imageAvg_mem((j-1)*RFILE_SIZE+1 : j*RFILE_SIZE) = imageB;
	end

	for k = 1 : IMAGE_SIZE/RFILE_SIZE
		imageB   = imageAvg_mem((k-1)*RFILE_SIZE+1 : k*RFILE_SIZE);
		imageAvg = imageB .* invM_vec; %! DPU<sat> [0,0]
		imageAvg_mem((k-1)*RFILE_SIZE+1 : k*RFILE_SIZE) = imageAvg;
	end
end
