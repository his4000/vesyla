% Memory testcase

% Features:
%  - Memory Write/Read
%  - Writing to memory and then reading from memory
%  - SRAM and Register File are in different columns (distance=3).
%  - Note that for memory write instructions, sv_results4_srm_x.txt should be checked.
%  - (0,0) -> (1,3)
%  - (1,3) -> (0,0)


BLOCK_SIZE = 32;


M = zeros(1, 256); %! MEM<> [0,3]

B = [0 : 31];     %! RFILE<> [0,0]

M(1:32) = B;

B = 0; %! DPU[0,0]

B = M(1:BLOCK_SIZE);
