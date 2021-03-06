% Memory testcase

% Features:
%  - Memory Read/Write
%  - Reading from memory followed by a memory write
%  - Note that for memory write instructions, sv_results4_srm_x.txt should be checked.
%  - (1,0) -> (0,4)
%  - (0,4) -> (1,0)

BLOCK_SIZE = 32;

M = [0 : 255]; 	  %! MEM<> [0,0]

A = zeros(1, BLOCK_SIZE); %! RFILE<> [0,4]

A = M(1:BLOCK_SIZE);

A = A + A; %! DPU[0,4]

M(33:64) = A;
