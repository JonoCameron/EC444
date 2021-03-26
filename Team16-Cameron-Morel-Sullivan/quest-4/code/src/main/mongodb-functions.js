/*
  Custom Node.js module that defines functions for MongoDB.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
// Server objects

const mongo = require('mongoose'),
Schema = mongo.Schema

var voter_schema
var vote_model
var db

exports.configDB = function(url_db){
  mongo.connect(url_db, {useUnifiedTopology: true, useNewUrlParser: true})
  db = mongo.connection
  db.on('error', (err) => {console.log(err)})

  voter_schema = new Schema({
    fob_id: Number,
    vote: String,
    time: Number
  })
  vote_model = mongo.model('votes', voter_schema)
}

exports.getCandidate = function(candidate, callback){
  return vote_model.find().where('vote').equals(candidate).exec((err, dbres) => {
    if(err) console.log(err)
    else {
      console.log(dbres)
      return callback(dbres)
    }
  })
}

exports.getAll = function(callback){
  return vote_model.find({}).exec((err, dbres) => {
    if(err) console.log(err)
    else {
      console.log(dbres)
      return callback(dbres)
    }
  })
}

exports.postVote = function(vote_data, callback){
  vote_model.find().where('fob_id').equals(vote_data.fob_id).exec((err, dbres) => {
    console.log(dbres)
    if(err) console.log(err)
    else if(dbres.length > 0) return callback('A')
    else {
      vote_model.create(vote_data)
      return(callback('R'))
    }
  })
}

exports.resetDB = function(callback){
  vote_model.deleteMany({ }, (err, dbres) => {
    if(err) console.log(err)
    else return callback('reset')
  })
}
