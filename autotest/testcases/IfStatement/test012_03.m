
% if-statement testcase

% Features:
%  - if-statement without explicit DPU allocation

A = [0 :10]; %! RFILE<> [0,0]
B = zeros(1, 5); %! RFILE<> [0,0]

if A(2) == B(1)
	B(1) = B(1) + A(2); %! DPU [0,0]
else
	B(2) = B(2) + A(3); %! DPU [0,0]
end
