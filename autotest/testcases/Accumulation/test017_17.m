
% Calculation with accumulation testcase

% Features:
%  - use sum() function to calculate a vector sum

A = [11:20]; %! RFILE<> [0,0]

A(1) = sum(A); %! DPU [0,0]
