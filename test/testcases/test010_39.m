% Memory testcase

% Features:
%  - Memory Write
%  - Writing two blocks of SRAM
%  - SRAM and Register File are in different columns.
%  - Note that for memory write instructions, sv_results4_srm_x.txt should be checked.
%  - (0,2) -> (1,4)

M = zeros(1, 64); %! MEM<> [0,4]
B = [0 : 31];     %! RFILE<> [0,2]

M(1:32) = B;
