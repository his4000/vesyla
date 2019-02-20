% Memory testcase

% Features:
%  - Memory Read/Write
%  - Writing to memory after reading from it
%  - SRAM and Register File are in different columns (distance=1).
%  - Note that for memory write instructions, sv_results4_srm_x.txt should be checked.
%  - (1,2) -> (0,1)
%  - (0,1) -> (1,2)

BLOCK_SIZE = 32;

M = [0 : 255]; 	  %! MEM<> [0,2]

A = zeros(1, BLOCK_SIZE); %! RFILE<> [0,1]

A = M(1:BLOCK_SIZE);

A = A + A; %! DPU[0,1]

M(1:BLOCK_SIZE) = A;
