function [t0, t1] = silago_dpu_2_macs(input1,kernel1,input2,kernel2)
	t0 = sum(input1 .* kernel1);
	t1 = sum(input2 .* kernel2);
end
