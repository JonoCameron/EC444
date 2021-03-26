var mongoDb         = require('mongodb');
var mongoClient     = mongoDb.MongoClient;
var dbname          = 'myDataBase';
var collectionName  = 'myCollection';
var url             = 'mongodb://localhost:27017/'+dbname;
var filename        = 'smoke.txt';
console.log('***************Process started');




mongoClient.connect(url,function(err,db){
    if(err){
        console.log('error on connection '+err);
    }
    else{
        console.log('***************Successfully connected to mongodb');
        var dbo = db.db("mydb");

/*        dbo.collection(collectionName).drop(function(err, delOK) {
          if (err) throw err;
          if (delOK) console.log("Collection deleted");
          db.close(); 
        });
*/
        var collection  = dbo.collection(collectionName);
        var fs          = require('fs');
        var readline    = require('readline');
        var stream      = require('stream');
        var instream    = fs.createReadStream(filename);
        var outstream   = new stream;
        var rl          = readline.createInterface(instream,outstream);

        console.log('***************Parsing, please wait ...');

        rl.on('line',function(line){
            try{
                var arr         = line.split('\t');
                var object   = { Time: arr[0], id: arr[1], Smoke: arr[2], Temp: arr[3] };
                //Parse them here
                //Example
//                object['Time'] = arr[0]; //Just an example

                dbo.collection(collectionName).insertOne(object, function(err, res) {
                  if (err) throw err;
                  console.log('1 document inserted');
                });
            }
            catch (err){
                console.log(err);
            }
        });

        rl.on('close',function(){
            db.close();
            console.log('***************completed');
        });
    }
});