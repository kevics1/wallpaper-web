import api from '../utils/http';

const MessageBoard = {
  id: Number,
  userId: Number,
  parentId: Number,
  content: String
}

export const getMessageBoards = (currentPage, pageSize) => {
  const params = {
    currentPage,
    pageSize
  }
  return api.get('/messageBoard/list', { params }).catch(err => { console.log(err) });
}

export const addMessageBoard = (MessageBoard) => {
  return api.post('/messageBoard/add', MessageBoard).catch(err => { console.log(err) });
}