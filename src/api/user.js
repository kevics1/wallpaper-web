import api from '../utils/http';

const User = {
  username: String,
  passwordHash: String
}

export const doLogin = (User) => {
  return api.post('/users/doLogin', User).catch(err => { console.log(err) });
};

export const doRegister = (User) => {
  return api.post('/users/doRegister', User).catch(err => { console.log(err) });
};
