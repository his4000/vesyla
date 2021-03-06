
% loop statement testcase (dynamic)

% Features:
%  - Three level nested loops
%  - Loop boundary with an increment bigger than one

HEIGHT = 4;
WIDTH = 4;
K_HEIGHT = HEIGHT / 2;
K_WIDTH = WIDTH / 2;

inp = [1:16]; %! RFILE<> [0,0]
kernel = [1 2 3 4]; %! RFILE<> [1,0]
acc = zeros(1, 1); %! RFILE<> [0,1]
temp = zeros(1, 1); %! RFILE<> [0,1]

for col = 1 : 2 : WIDTH-1
	for i = 1 : K_HEIGHT
		for j = 1 : K_WIDTH
			temp(1) = inp((i-1)*WIDTH + col + j - 1) .* kernel((i-1)*K_WIDTH + j); %! DPU[0,0]
			acc(1) = acc(1) + temp(1); %! DPU[0,1]
		end
	end
end
