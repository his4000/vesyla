
% Min/Max Calculation

% Features:
%  - Minimum calculation

A = [3:34]; 	  %! RFILE<> [0,0]
B = zeros(1, 1);  %! RFILE<> [0,1]

B = min(A); %! DPU [0,0]
