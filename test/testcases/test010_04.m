
% Memory testcase

% Features:
%  - Memory Read
%  - Reading two blocks of SRAM
%  - SRAM and Register File are in different columns.

M = [0 : 63]; 	  %! MEM<> [0,0]
B = zeros(1, 32); %! RFILE<> [0,1]
X = zeros(1, 32); %! RFILE<> [1,0]

B = M(1:32);

X = B + B; %! DPU[0,0]
