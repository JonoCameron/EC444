const axios = require('axios')

axios.defaults.headers.common = {
    "Content-Type": "application/json"
  }

var data =  {
    fob_id: 2,
    vote: 'Test Tester',
    time: 200
}
axios.post('http://localhost:8080/vote', data).then((res) => {
        console.log(res.data)
    }, (err) => {
        console.log('Error')
    })
