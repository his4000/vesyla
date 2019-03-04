function o = silago_dpu_scale_mul(x, y, r)
	o = r*x.*y;
	if o>32767
		o = 32767;
	elseif o<-32768
		o = -32768;
	end
