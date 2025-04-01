import api from '../utils/http';

export const getWallpapers = (currentPage, pageSize, categoryId, keyword) => {
  const params = {
    currentPage: currentPage,
    pageSize: pageSize,
    categoryId: categoryId,
    keyword: keyword
  };
  return api.get('/wallpapers/list', { params }).catch(err => {
    console.log(err);
  });
};

export const getWallpaperDetail = (id) => {
  return api.get(`/wallpapers/getWallpaperDetail`, { params: { id } }).catch(err => {
    console.log(err);
  });
};

export const getCarousels = () => {
  return api.get('/wallpapers/carousels').catch(err => {
    console.log(err);
  });
};

export const operateWallpaper = ({ wallpaperId, type }) => {
  const formData = new FormData();
  formData.append('wallpaperId', wallpaperId); // Long 类型转换为字符串
  formData.append('type', type); // Integer 类型转换为字符串

  return api.post('/wallpapers/operate', formData).catch(err => {
    console.log(err);
  });
};

export const saveWallpaper = ({ title, categoryIds, file }) => {
  const formData = new FormData();
  formData.append('title', title);
  formData.append('categoryIds', categoryIds); // 将数组转换为逗号分隔字符串
  formData.append('file', file);

  return api.post('/wallpapers/save', formData).catch(err => {
    console.log(err);
  })
}

export const getCategory = (currentPage, pageSize, id, categoryName, parentId) => {
  const params = {
    currentPage: currentPage,
    pageSize: pageSize,
    id: id,
    name: categoryName,
    parentId: parentId
  };
  return api.get('/wallpapers/category', { params }).catch(err => {
    console.log(err);
  })
}