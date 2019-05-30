function [t] = quat_mult(a,b)
    t0 = a(1)*b(1) - dot(a(2:4),b(2:4));
    t1 = a(1)*b(2:4) + b(1)*a(2:4) + cross(a(2:4),b(2:4));
    t = [t0, t1];
end

