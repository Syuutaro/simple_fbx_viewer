#ifndef MATRIX_HPP
#define MATRIX_HPP

#define COLUMN_MAJOR_MATRIX

#include <initializer_list>
#include <cmath>

//vector
template <typename T,size_t N>
class Vector{
private:
    T m_c[N];
public:
    //constructor
    Vector():m_c{0}{}
    Vector(std::initializer_list<T> list):m_c{0}{
        if (N == list.size()){
            const T* ite = list.begin();
            for (size_t i = 0;i < N;i++){
                m_c[i] = *ite;
                ++ite;
            }
        }
    }
    Vector(const Vector<T,N-1>& v,const T& s){
        for (size_t i = 0;i < N-1;i++){
            m_c[i] = v[i];
        }
        m_c[N-1] = s;
    }
    Vector(const Vector<T,N+1>& v){
        for (size_t i = 0;i < N;i++){
            m_c[i] = v[i];
        }
    }

    //substitution
    const Vector<T,N>& operator=(const Vector<T,N-1>& v){
        for (size_t i = 0;i < N-1;i++){
            m_c[i] = v[i];
        }
        m_c[N-1] = 0;
        return *this;
    }
    const Vector<T,N>& operator=(const Vector<T,N+1>& v){
        for (size_t i = 0;i < N;i++){
            m_c[i] = v[i];
        }
        return *this;
    }

    //compare
    bool operator==(const Vector<T,N>& v) const{
        for (size_t i = 0;i < N;i++){
            if (m_c[i] != v[i]){
                return false;
            }
        }
        return true;
    }
    bool operator!=(const Vector<T,N>& v) const{
        for (size_t i = 0;i < N;i++){
            if (m_c[i] != v[i]){
                return true;
            }
        }
        return false;
    }

    //member access
    const T& operator[](size_t i) const{
        return m_c[i];
    }
    T& operator[](size_t i){
        return m_c[i];
    }

    //operation
    //vector_vector
    Vector<T,N> operator+(const Vector<T,N>& v) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]+v[i];
        }
        return r;
    }
    Vector<T,N> operator-(const Vector<T,N>& v) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]-v[i];
        }
        return r;
    }
    Vector<T,N> operator*(const Vector<T,N>& v) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]*v[i];
        }
        return r;
    }
    Vector<T,N> operator/(const Vector<T,N>& v) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]/v[i];
        }
        return r;
    }
    const Vector<T,N>& operator+=(const Vector<T,N>& v){
        for (size_t i = 0;i < N;i++){
            m_c[i] += v[i];
        }
        return *this;
    }
    const Vector<T,N>& operator-=(const Vector<T,N>& v){
        for (size_t i = 0;i < N;i++){
            m_c[i] -= v[i];
        }
        return *this;
    }
    const Vector<T,N>& operator*=(const Vector<T,N>& v){
        for (size_t i = 0;i < N;i++){
            m_c[i] *= v[i];
        }
        return *this;
    }
    const Vector<T,N>& operator/=(const Vector<T,N>& v){
        for (size_t i = 0;i < N;i++){
            m_c[i] /= v[i];
        }
        return *this;
    }

    //vector_scalar
    Vector<T,N> operator+(const T& s) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]+s;
        }
        return r;
    }
    Vector<T,N> operator-(const T& s) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]-s;
        }
        return r;
    }
    Vector<T,N> operator*(const T& s) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]*s;
        }
        return r;
    }
    Vector<T,N> operator/(const T& s) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = m_c[i]/s;
        }
        return r;
    }
    const Vector<T,N>& operator+=(const T& s){
        for (size_t i = 0;i < N;i++){
            m_c[i] += s;
        }
        return *this;
    }
    const Vector<T,N>& operator-=(const T& s){
        for (size_t i = 0;i < N;i++){
            m_c[i] -= s;
        }
        return *this;
    }
    const Vector<T,N>& operator*=(const T& s){
        for (size_t i = 0;i < N;i++){
            m_c[i] *= s;
        }
        return *this;
    }
    const Vector<T,N>& operator/=(const T& s){
        for (size_t i = 0;i < N;i++){
            m_c[i] /= s;
        }
        return *this;
    }

};

//utility vector function
template <typename T,size_t N>
T dot(const Vector<T,N>& v1,const Vector<T,N>& v2){
    T r = 0;
    for (size_t i = 0;i < N;i++){
        r += v1[i]*v2[i];
    }
    return r;
}

template <typename T,size_t N>
T length(const Vector<T,N>& v){
    return std::sqrt(dot<T,N>(v,v));
}

template <typename T,size_t N>
Vector<T,N> normalize(const Vector<T,N>& v){
    T l = length<T,N>(v);
    if (l != 0){
        return v/l;
    }else{
        return Vector<T,N>();
    }
}

template <typename T>
Vector<T,3> cross(const Vector<T,3>& v1,const Vector<T,3>& v2){
    Vector<T,3> r;
    r[0] = v1[1]*v2[2]-v1[2]*v2[1];
    r[1] = v1[2]*v2[0]-v1[0]*v2[2];
    r[2] = v1[0]*v2[1]-v1[1]*v2[0];
    return r;
}




//matrix
template <typename T,size_t N>
class Matrix{
private:
    T m_c[N*N];
public:
    Matrix():m_c{0}{}
    
    //member access
    void SetComponent(size_t row,size_t column,const T& s){
        size_t idx = GetIndex(row,column);
        m_c[idx] = s;
    }
    const T& GetComponent(size_t row,size_t column) const{
        size_t idx = GetIndex(row,column);
        return m_c[idx];
    }
    
    void SetRow(size_t row,const Vector<T,N>& v){
        for (size_t i = 0;i < N;i++){
            size_t idx = GetIndex(row,i);
            m_c[idx] = v[i];
        }
    }
    Vector<T,N> GetRow(size_t row) const{
        Vector<T,N> v;
        for (size_t i = 0;i < N;i++){
            size_t idx = GetIndex(row,i);
            v[i] = m_c[idx];
        }
        return v;
    }
    
    void SetColumn(size_t column,const Vector<T,N>& v){
        for (size_t i = 0;i < N;i++){
            size_t idx = GetIndex(i,column);
            m_c[idx] = v[i];
        }
    }
    Vector<T,N> GetColumn(size_t column) const{
        Vector<T,N> v;
        for (size_t i = 0;i < N;i++){
            size_t idx = GetIndex(i,column);
            v[i] = m_c[idx];
        }
        return v;
    }
    
    //operation
    //matrix_matrix
    Matrix<T,N> operator+(const Matrix<T,N>& m) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(i,j)+m.GetComponent(i,j));
            }
        }
        return r;
    }
    Matrix<T,N> operator-(const Matrix<T,N>& m) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(i,j)-m.GetComponent(i,j));
            }
        }
        return r;
    }
    Matrix<T,N> operator*(const Matrix<T,N>& m) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,dot(GetRow(i),m.GetColumn(j)));
            }
        }
        return r;
    }
    const Matrix<T,N>& operator+=(const Matrix<T,N>& m){
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                SetComponent(i,j,GetComponent(i,j)+m.GetComponent(i,j));
            }
        }
        return *this;
    }
    const Matrix<T,N>& operator-=(const Matrix<T,N>& m){
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                SetComponent(i,j,GetComponent(i,j)-m.GetComponent(i,j));
            }
        }
        return *this;
    }

    //matrix_vector
    Vector<T,N> operator*(const Vector<T,N>& v) const{
        Vector<T,N> r;
        for (size_t i = 0;i < N;i++){
            r[i] = dot(GetRow(i),v);
        }
        return r;
    }

    //matrix_scalar
    Matrix<T,N> operator+(const T& s) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(i,j)+s);
            }
        }
        return r;
    }
    Matrix<T,N> operator-(const T& s) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(i,j)-s);
            }
        }
        return r;
    }
    Matrix<T,N> operator*(const T& s) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(i,j)*s);
            }
        }
        return r;
    }
    Matrix<T,N> operator/(const T& s) const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(i,j)/s);
            }
        }
        return r;
    }
    const Matrix<T,N>& operator+=(const T& s){
        for (size_t i = 0;i < N*N;i++){
            m_c[i] += s;
        }
        return *this;
    }
    const Matrix<T,N>& operator-=(const T& s){
        for (size_t i = 0;i < N*N;i++){
            m_c[i] -= s;
        }
        return *this;
    }
    const Matrix<T,N>& operator*=(const T& s){
        for (size_t i = 0;i < N*N;i++){
            m_c[i] *= s;
        }
        return *this;
    }
    const Matrix<T,N>& operator/=(const T& s){
        for (size_t i = 0;i < N*N;i++){
            m_c[i] /= s;
        }
        return *this;
    }
    
    Matrix<T,N> Transpose() const{
        Matrix<T,N> r;
        for (size_t i = 0;i < N;i++){
            for (size_t j = 0;j < N;j++){
                r.SetComponent(i,j,GetComponent(j,i));
            }
        }
        return r;
    }
private:
    size_t GetIndex(size_t row,size_t column) const{
#ifdef COLUMN_MAJOR_MATRIX
        return N*column+row;
#else
        return N*row+column;
#endif
    }
};

#endif //MATRIX_HPP
