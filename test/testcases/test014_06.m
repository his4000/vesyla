
% loop statement testcase (dynamic)

% Features:
%  - Loop boundary with a negative increment

A = [0 :11]; %! RFILE<> [0,0]
B = [20:34]; %! RFILE<> [0,0]
C = [2 :24]; %! RFILE<> [1,1]
D = [0 : 5]; %! RFILE<> [1,1]

for j = 6 : -2 : 2
	A(j) = B(2*j+1) + C(j); %! DPU [0,0]
	D(j) = A(j) + D(j); %! DPU [1,1]
end
