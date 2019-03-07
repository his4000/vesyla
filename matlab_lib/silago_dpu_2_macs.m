function t = silago_dpu_2_macs(input1,kernel1,input2,kernel2)
	t(1) = sum(input1 .* kernel1);
	t(2) = sum(input2 .* kernel2);
end
