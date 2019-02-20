
% Calculation with accumulation testcase

% Features:
%  - Calculation using Preserving-MAC operation inside nested loops

A      = fi([0.012024 0.012024 0.012146 0.012421 0.012665 0.012817 0.011780 0.011383 0.011627 0.012817 0.013733 0.014130 0.014374 0.014893 0.016998 0.012146 0.006653 0.004700 0.006134 0.007721 0.007050 0.005493 0.006134 0.005890 0.005371 0.004974 0.004700 0.004700 0.004822 0.004974 0.006134 0.005890], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0,0]
B      = fi([0.006012 0.006409 0.006409 0.006134 0.005615 0.005096 0.004456 0.004181 0.004456 0.005096 0.005096 0.004456 0.004181 0.004456 0.005737 0.005737 0.005737 0.005737 0.005737 0.005737 0.005737 0.005737 0.004822 0.004822 0.004974 0.005219 0.005371 0.005615 0.005737 0.005890 0.005493 0.005493], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0,1]
result = fi(zeros(1,16), 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0,2]

for i = 1 : 16
	for j = 1 : 3
	  result(i) = result(i) + sum(fi(A .* B, 1, 16, 15, 'RoundingMethod', 'Floor')); %! DPU<sat> [0,0]
	end
end
