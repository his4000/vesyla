
%  Dimensionality restoration testcase

% Features:
%  - Dimensionality restoration
%  - We basically have to muliply A [n^2()rows*M(columns)]matrix with V=M(rows)*1(column) vectors to get N^2
%  - Memory memory block A contains 32 rows of matrix A it will multiply and
%    restore it one by one in n^2/32 steps.

M = 8; % Total images
N = 16; % Image Dimension
IMAGE_SIZE = N*N; % Image size
MEM_BLOCK_SIZE = 16; % Number of words in a memory block

% Memory declarations
% Memory memory block A contains 32 rows of matrix A it will multiply and
% restore it one by one in n^2/32 steps.
imageA_mem = fi([0.103601 0.003601 0.003662 0.003723 0.003815 0.003845 0.003540 0.003418 0.003479 0.003845 0.004120 0.004242 0.004303 0.004456 0.005096 0.003662 0.002014 0.001404 0.001831 0.002319 0.002106 0.001648 0.001831 0.001770 0.001617 0.001495 0.001404 0.001404 0.001465 0.001495 0.001831 0.001770 0.001740 0.001740 0.001801 0.001831 0.001801 0.001740 0.001556 0.001648 0.001801 0.001923 0.001923 0.001831 0.001678 0.001526 0.001343 0.001251 0.001343 0.001526 0.001526 0.001343 0.001251 0.001343 0.001740 0.001740 0.001740 0.001740 0.001740 0.001740 0.001740 0.001740 0.001465 0.001465 0.001495 0.001556 0.001617 0.001678 0.001740 0.001770 0.001648 0.001648 0.001678 0.001770 0.001801 0.001831 0.001923 0.001923 0.002075 0.002106 0.002167 0.002228 0.002319 0.002380 0.002472 0.002502 0.002747 0.002747 0.003052 0.002228 0.010010 0.009583 0.010010 0.009857 0.002563 0.002655 0.001801 0.002289 0.001923 0.002075 0.001953 0.001953 0.002014 0.002045 0.002106 0.002167 0.002197 0.002197 0.002228 0.002197 0.002167 0.002106 0.002075 0.002045 0.002014 0.001953 0.002106 0.002167 0.002167 0.002167 0.002197 0.002228 0.002228 0.002228 0.002441 0.002441 0.002472 0.002502 0.002563 0.002594 0.002625 0.002655 0.002655 0.002716 0.002747 0.002777 0.002869 0.002899 0.002930 0.002930 0.002869 0.002930 0.003113 0.003265 0.003326 0.003326 0.003296 0.003204 0.003448 0.003479 0.003479 0.003540 0.003571 0.003571 0.003601 0.003601 0.003693 0.003693 0.003693 0.003693 0.003693 0.003693 0.003693 0.003693 0.003845 0.003845 0.003815 0.003815 0.003754 0.003754 0.003723 0.003723 0.003662 0.003662 0.003662 0.003662 0.003662 0.003662 0.003662 0.003662 0.003448 0.003418 0.003387 0.003326 0.003326 0.003387 0.003448 0.003479 0.003448 0.003601 0.003723 0.003815 0.004120 0.004517 0.004639 0.004517 0.003937 0.002441 0.001221 0.001373 0.002045 0.002106 0.001740 0.001373 0.001801 0.001801 0.001770 0.001740 0.001740 0.001740 0.001740 0.001740 0.001404 0.001343 0.001251 0.001282 0.001373 0.001404 0.001343 0.001282 0.001251 0.001099 0.001007 0.001099 0.001343 0.001465 0.001343 0.001221 0.001495 0.001373 0.001373 0.001526 0.001526 0.001373 0.001373 0.001495 0.001404 0.001404 0.001404 0.001404 0.001404 0.001404 0.001404 0.001404 0.001495 0.001526 0.001556 0.001648 0.001740 0.001770 0.001801 0.001831 0.001801 0.001801 0.001831 0.001892], 1, 16, 15, 'RoundingMethod', 'Floor'); %! MEM<> [0,0]
imageV_mem = fi([0.012024 0.012024 0.012146 0.012421 0.012665 0.012817 0.011780 0.011383 0.011627 0.012817 0.013733 0.014130 0.014374 0.014893 0.016998 0.012146], 1, 16, 15, 'RoundingMethod', 'Floor'); %! MEM<> [0,1]
actual_mem = fi(zeros(1, IMAGE_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! MEM<> [0,2]

% RegFile declarations
imageA = fi(zeros(1,MEM_BLOCK_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<input> [0,0]
imageV = fi(zeros(1,MEM_BLOCK_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<input> [0,1]
actual = fi(zeros(1,MEM_BLOCK_SIZE), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<output> [0,2]

imageV = imageV_mem;

for i = 1 : IMAGE_SIZE/MEM_BLOCK_SIZE
	for j = 1 : MEM_BLOCK_SIZE
		imageA   = imageA_mem((j-1)*MEM_BLOCK_SIZE+1 : j*MEM_BLOCK_SIZE);
		actual(j) = sum(fi(imageA .* imageV, 1, 16, 15, 'RoundingMethod', 'Floor')); %! DPU<sat> [0,0]
	end
	actual_mem((j-1)*MEM_BLOCK_SIZE+1 : j*MEM_BLOCK_SIZE) = actual;
end
