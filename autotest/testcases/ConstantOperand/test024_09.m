
% Constant operation testcase

% Features:
%  - Add with constant vector operation

A      = fi([0.012024 0.012024 0.012146 0.012421 0.012665 0.012817 0.011780 0.011383 0.011627 0.012817 0.013733 0.014130 0.014374 0.014893 0.016998 0.012146 0.006653 0.004700 0.006134 0.007721 0.007050 0.005493 0.006134 0.005890 0.005371 0.004974 0.004700 0.004700 0.004822 0.004974 0.006134 0.005890], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0,0]
result = fi([0 0 0 0], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0,2]

result = A(1:4) + fi(0.2, 1, 8, 7, 'RoundingMethod', 'Floor'); %! DPU<sat> [0,0]
