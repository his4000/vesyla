function [t0, t1] = silago_dpu_2_mult(input0, input1, input2, input3)
        t0 = input0 .* input1;
        t1 = input2 .* input3;
end
