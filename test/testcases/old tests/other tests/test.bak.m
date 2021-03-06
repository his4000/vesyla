
% 2D Convolution

HEIGHT = 4;
WIDTH = 4;
K_HEIGHT = HEIGHT / 2;
K_WIDTH = WIDTH / 2;

inp = [1:16]; %! RFILE<> [0,0]
kernel = [1:10]; %! RFILE<> [1,0]
out = zeros(1, 16); %! RFILE<> [1,0]
acc = zeros(1, 1); %! RFILE<> [0,1]
temp = zeros(1, 1); %! RFILE<> [0,1]

for row = 1 : 3
	for col = 1 : 4

		for i = 1 : col+1
			for j = 1 : K_WIDTH
			%	temp(1) = inp(row + i + col + j - 1) .* kernel((i-1)*K_WIDTH + j); %! DPU[0,0]
			%	acc(1) = acc(1) + temp(1); %! DPU[0,1]
				out(i + j) = inp(i) .* kernel(j); %! DPU[0,0]
			end
		end

	%	out((row-1)*WIDTH + col) = acc(1);
	end
end

