import api from '../utils/http';

const Comment = {
  // 用户ID
  userId: Number,
  // 壁纸ID
  wallpaperId: Number,
  // 评论内容
  content: String,
}

export const getComments = (currentPage, pageSize, wallpaperId) => {
  const params = {
    currentPage: currentPage,
    pageSize: pageSize,
    wallpaperId: wallpaperId
  };
  return api.get('/comments/getComments', { params }).catch(err => { console.log(err) });
};

export const sendComment = (Comment) => {
  // 使用axios或其他HTTP客户端发起POST请求
  return api.post('/comments/add', Comment)
    .catch((err) => {
      console.error('发送评论时发生错误:', err);
    });
};