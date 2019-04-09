function [t0, t1] = silago_dpu_double_2_max(input0,input1,input2,input3)
        t0 = max(input0, input1);
        t1 = max(input2, input3);
end
