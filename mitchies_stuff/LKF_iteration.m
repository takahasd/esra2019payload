function [r] = LKF_iteration(r0, av0, dav, dt, t0, tf)

    r = r0;
    av = av0;
    
    for k = t0:dt:tf
        r = r + dt*cross(av, r);
        av = av + dt*dav;
    end
end