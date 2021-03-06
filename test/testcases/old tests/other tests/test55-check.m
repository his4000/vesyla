
% parallelization

A = [0:31]; %! RFILE<even> [0,1:2]
B = [2 4 6 8 2 4 6 8 2 4 6 8 2 4 6 8]; %! RFILE<> [0,1]
C = zeros(1, 32); %! RFILE<even> [0,0:1]
D = zeros(1, 16); %! RFILE<even> [1,1]

C = A + B; %! DPU[0,0:1]

for i = 1 : 32
	E(1) = E(1) + C(i); %! DPU[1,0]
end

D(2:10) = A(2:10) + B(2:10); %! DPU[1,1]

