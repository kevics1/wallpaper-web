import axios from 'axios';
import { Message } from "element-ui";
import { showLoginModal } from '../utils/LoginModal.js';
import { BASE_API_URL } from './baseUrl.js';

// 创建axios实例
const instance = axios.create({
    baseURL: BASE_API_URL,
    timeout: 10000, // 超时时间
    withCredentials: false
})

instance.interceptors.request.use(config => {
    const satoken = localStorage.getItem('satoken');
    if (satoken) {
        config.headers['satoken'] = `Bearer ${satoken}`;
    }
    return config;
}, error => {
    return Promise.reject(error);
});

// 响应拦截
instance.interceptors.response.use(response => {
    console.log(response)
    if (response.status === 200) {
        const resCode = response.data.code;
        if (resCode == 400200) {
            showLoginModal();
        }


        return response.data;
    } else {
        // 响应失败
        Message.error(response.data.message);
        return Promise.reject(response.data);
    }
}, error => {
    console.log(error)
    if (!error.response) {
        console.error('Network Error:', error);
        Message.error('网络连接异常，请检查网络后重试');
    } else {
        const errorMessage = error.response.data.message || '请求失败，请稍后再试';
        Message.error(errorMessage);
    }

    return Promise.reject(error);
});

export default instance;