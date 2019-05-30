function [w,v_new] = quat_rot( v, theta, r_rot )

   r_rot = r_rot/norm(r_rot);
   
   q0 = cos(theta/2);
   q1 = sin(theta/2) * r_rot;
   
   v_ = [0,v];
   
   q = [q0, q1];
   q_inv = [q0, -1*q1];
   
   v_ = quat_mult(v_, q_inv);
   
   ans = quat_mult(q, v_);
   
   w = ans(1);
   v_new = ans(2:4);
end
    
    
    