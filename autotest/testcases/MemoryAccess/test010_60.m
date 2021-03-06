% Memory testcase

% Features:
%  - Memory Read/Write
%  - Writing to memory after reading from it
%  - SRAM and Register File are in different columns (distance=4).
%  - Note that for memory write instructions, sv_results4_srm_x.txt should be checked.
%  - (0,0) -> (1,0)
%  - (0,0) -> (1,4)

BLOCK_SIZE = 32;

M = [0 : 255]; 	  %! MEM<> [0,0]

A = zeros(1, BLOCK_SIZE); %! RFILE<> [0,0]

A = M(1:BLOCK_SIZE);

A = A + A; %! DPU[0,0]

M1 = zeros(1, 256);  %! MEM<> [0,4]

M1(1:BLOCK_SIZE) = A;





